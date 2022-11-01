// workthread.cpp
// workthread type: interactive

// TODO: Pokud potrebujes pouzit previous (prechod na predchozi): "Udelej si sam" :))

#include <iostream>
#include <QtCore/qglobal.h>

#include "include/vtapi.h"

#include "workthread.h"

// OpenCV header files
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

String itoa(int i)
{
   std::stringstream ss;
   ss << i;
   return ss.str();
}



Mat& paintTrack(const int track, const int frame, const std::vector<int>& frames, const std::vector<PGpoint>& positions, const std::vector<PGpoint>& sizes, cv::Mat& img) {
    // assert painting ability
    if (img.empty()) return img;

    // assert lenght size
    size_t length = frames.size();
    if (length != positions.size() || length != sizes.size()) {
        String text = "NO TRAJECTORY?!";
        cv::putText(img, text, cv::Point(400,16), 0, 0.5, Scalar::all(0), 3);
        cv::putText(img, text, cv::Point(400,16), 0, 0.5, Scalar(0, 0, 255), 1);
    }

    // trajectory goes from white to yellow :)
    for (size_t i = 0; i < length; i++) {
        int b = 255.0 - (255.0/(length - 1))*i;
        if (i > 0) {
            cv::line(img, cv::Point(positions[i-1].x, positions[i-1].y), cv::Point(positions[i].x, positions[i].y), Scalar(b, 255, 255), 1);
        }

        // draw the bounding rectangle
        if (frames[i] == frame) {
            cv::rectangle(img, cv::Point(positions[i].x - sizes[i].x/2, positions[i].y - sizes[i].y/2), cv::Point(positions[i].x + sizes[i].x/2, positions[i].y + sizes[i].y/2), Scalar(0, 255, 0), 1);
            // cv::putText(img, itoa(track), cv::Point(positions[i].x - sizes[i].x/2, positions[i].y - sizes[i].y/2 - 5), 0, 0.4, Scalar::all(0), 3);
            cv::putText(img, itoa(track), cv::Point(positions[i].x - sizes[i].x/2, positions[i].y - sizes[i].y/2 - 4), 0, 0.4, Scalar(0, 255, 0), 1);
        }
    }

    return img;
}



WorkThread::WorkThread(int argc, char** argv, QObject *parent) : QThread(parent) {
    doom = false;
    plusClicked = false;
    timerTick = false;
    nextClicked = false;
    sequence = NULL;

    // init VTApi
    vtapi = new VTApi(argc, argv);
    dataset = vtapi->newDataset(vtapi->commons->getDataset());
}

// TODO: Jozin: po kliknuti toggle/zastavit!!!!
/// TODO: po kolecku mysi prepnout focus trajektorie :)
void WorkThread::run() {
    // CUMSEM: takto se nastavuje event, ktery se anotuje... ono to moc jinak nejde zatim // TODO:)
    String event = "personruns";
    // personruns celltoear objectput embrace pointing elevatornoentry opposingflow 
    // peoplemeet peoplesplitup takepicture (not taken :)

    // tohle jsou casy prehravani
    int frame = 0;
    int t1 = 0;
    int t2 = 0;
    
    // tohle 3-4 je 1 trajektorie
    int track;
    std::vector<int>* frames = NULL;
    std::vector<PGpoint>* positions = NULL;
    std::vector<PGpoint>* sizes = NULL;

    // takhle se pak zeptam na serazeni sequenci, ve kterych se to bude rucne anotovat
    KeyValues* sequence = new KeyValues(*dataset, "top_" + event); //  + "_dryrun"
    destruct(sequence->select);
    sequence->select = new Select(*sequence, "SELECT seqname, sum("+ event +"), count("+ event +") \n"
                     + " FROM " + vtapi->commons->getDataset() + ".top_"+ event +"\n"  // _dryrun
                     + " GROUP BY seqname\n"
                     + " ORDER BY sum("+ event +") DESC;\n");

    while (sequence->next()) {
        // init VideoCapture // EvalSIN12
        String filename = dataset->baseLocation + "i-LIDS/Gatwick/bgfg/" + sequence->getString("seqname") + ".BGFG.avi";
        videoCapture = VideoCapture(filename); // FIXME cteni ze souboru
        if (!videoCapture.isOpened()) {
            qDebug() << filename.c_str();
            emit eventLabelChanged( QString("Toz to je divne, kde mas todle video :(") );
            t1 = t2 = 0;
        }


        // FIXME: Vojta by jako navrh na diplomku mohl udelat ty selections a JOINy :) a jeste ze srandy i s textovymi daty (header)
        // to, co dedi z Interval si musi najit primarni klic a pokusit se to propojit pres vsechny polozky toho klice
        // annotation = new Interval(*sequence, "annotations");
        // annotation->select->whereString("event", "OpposingFlow");
        annotation = new Interval(*sequence, "top_" + event);   // _drurun... in fact, there are just plain KeyValues
        destruct(annotation->select);
        annotation->select = new Select(*annotation, String("SELECT e.seqname, t1, t2, e.track, frames, positions, sizes\n")
                           + " FROM " + vtapi->commons->getDataset() + ".top_"+ event +" AS e JOIN " + vtapi->commons->getDataset() + ".tracks AS t\n" // _dryrun
                           + "      ON (e.seqname = t.seqname AND e.track = t.track)\n"
                           + " WHERE e.seqname = '"+ sequence->getString("seqname") +"'\n"
                           + " ORDER BY t1, e.track;");
        // TODO: do top_XY pridat t1 a t2!

        if (annotation->next() != NULL) {
            emit eventLabelChanged( event.c_str() ); // toz, todle akorat ze zvyku

            // trajectory
            track = annotation->getInt("track");
            frames = annotation->getIntV("frames");
            positions = annotation->getPointV("positions");
            sizes = annotation->getPointV("sizes");

            // video time
            t1 = annotation->getStartTime();
            t2 = annotation->getEndTime();
            emit eventPlusTime( spreadShot(t1, t2) );
	    
            frame = t1;
            videoCapture.set(CV_CAP_PROP_POS_FRAMES, t1);
        }

        // start the timer
        connect(&timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
        timer.start(30); // 1000/25 [ms] = 40

        // cyklus paralelniho zpracovani                                            // while (true)
        while (true) {
            // be doomed
            if(doom) break;

            // dela neco, kdyz je neco stisknute...
            if (nextClicked) {
                nextClicked = false;

                // uloz co prislo od toho/te na zidli
                int verify = atoi(text.toLocal8Bit().constData());
                annotation->update = new Update(*annotation);
                annotation->update->keyInt("verified", verify);
                annotation->update->where = "seqname = '"+ sequence->getString("seqname") +"' AND track="+ itoa(track);
                // tohle se vodpali hned pri zavolani next(), takze pohoda :)

                // jestli se NEda pretocit v tomdle videu... ?
                if (annotation->next() != NULL) {
                    // trajectory
                    track = annotation->getInt("track");
                    frames = annotation->getIntV("frames");
                    positions = annotation->getPointV("positions");
                    sizes = annotation->getPointV("sizes");

                    // video time
                    t1 = annotation->getStartTime();
                    t2 = annotation->getEndTime();
                    emit eventPlusTime( spreadShot(t1, t2) );
		    
                    frame = t1;
                    videoCapture.set(CV_CAP_PROP_POS_FRAMES, t1);
                }
                // to je konec :(
                else {
                    emit eventLabelChanged( QString("Tadydadyda, to je konec!!!")  );
                    break;
                }

                // jeste pro sichr spustit timer
                if(!timer.isActive()) {
                    timer.start();
                    timerTick = true;
                }
            } // if (nextClicked)

            // tohle je, jestli chce zvetsit rozsah
            if (plusClicked) {
                plusClicked = false;
                int t = spreadShot(t1, t2);
                t1 -= t/2;
                t2 += t - t/2;

                emit eventPlusTime( spreadShot(t1, t2) );
            }
             
            if (replayClicked) {
              replayClicked = false;
              videoCapture.set(CV_CAP_PROP_POS_FRAMES, t1);
            }

            // nebo dela, kdyz timer rekne, ze se da neco vykreslit
            if (timerTick) {
                timerTick = false;

                Mat vc;
                videoCapture >> vc; // nacteni obrazku z kamery, videa
                if (!vc.empty()) {
                    frame = videoCapture.get(CV_CAP_PROP_POS_FRAMES);
                    paintTrack(track, frame, *frames, *positions, *sizes, vc);
                    cv::putText(vc, sequence->getString("seqname") + " " + itoa(frame), cv::Point(5,16), 0, 0.5, Scalar::all(0), 3);
                    cv::putText(vc, sequence->getString("seqname") + " " + itoa(frame), cv::Point(5,16), 0, 0.5, Scalar(0, 255, 255), 1);

                    resize(vc, mat, Size(), 1.0, 1.0, INTER_LINEAR); // toz mi ho trocha zvets
                }

                image = MatToQImage(mat);
                emit newFrame(image /* ... */);

                // cykli jak blbej, dokud to mezi zidlou a klavesnici neklikne na "Dalsi"
                if (videoCapture.get(CV_CAP_PROP_POS_AVI_RATIO) == 1 || frame > t2) {
                    videoCapture.set(CV_CAP_PROP_POS_FRAMES, t1);
                }

            } // if (timerTick)

            // tohle je tu, aby to tak zase moc nezahrivalo procesor, akorat to obcas nevali :(
            msleep(10);
        } // while true ===
    } // while sequence

    emit eventLabelChanged( QString("Tadydadyda, to je definitivne konec!!!")  );
} // run()


WorkThread::~WorkThread() {
    timer.stop();
    videoCapture.release();

    destruct(annotation);
    destruct(sequence);
    destruct(dataset);
    
    destruct(vtapi);
}

// defaultne se budeme tocit dopredu :)
void WorkThread::directionButtonClicked(const QString& str, const int direction = 1) {
    if (direction == -1) {
      previousClicked = true;
      std::cout << "Pokud potrebujes pouzit previous (prechod na predchozi): \"Udelej si sam\" :))" << std::endl;
    }
    else if (direction == 1) {
      nextClicked = true;
    }
    else {
       // Takovy smer tu nemam
    }
  

    this->text = str;
}

void WorkThread::timerUpdate() {
    timerTick = true;
}

void WorkThread::togglePause() {
    if(timer.isActive()) {
        timer.stop();
        timerTick = false;
    }
    else {
        timer.start();
        timerTick = true;
    }
}

void WorkThread::plusTime() {
    plusClicked = true;
}

void WorkThread::replay() {
  replayClicked = true;
}

void WorkThread::killMe() {
    doom = true;
}

int WorkThread::spreadShot(int t1, int t2) {
  int t = t2 - t1;
  // Vlczech: uz zadne dalsi staticke sekvence videi o velikosti 1 framu, ktere se nedaji pomoci "+" rozsirit
  if (t < 20) {
    t = 20;
  }
  else if (t > 250) {
    t = 250;
  }
  return t;
}