// workthread.cpp
// workthread type: annotator

#include <iostream>
#include <QtCore/qglobal.h>

#include "include/vtapi.h"

#include "workthread.h"
//#include "MatToQImage.h"


WorkThread::WorkThread(int argc, char** argv, QObject *parent) : QThread(parent) {
    doom = false;
    replayClicked = false;
    plusClicked = false;
    timerTick = false;
    previousClicked = false;
    nextClicked = false;

    // init VTApi
    vtapi = new VTApi(argc, argv);
    dataset = vtapi->newDataset(vtapi->commons->getDataset());
}

// TODO: Jozin: po kliknuti toggle/zastavit!!!!
void WorkThread::run() {
    // tohle jsou casy prehravani
    int t1 = 0;
    int t2 = 0;

    // todle je pro inserty
    Interval* annTrack;

    // CUCSEM: nastav ve vtapi.conf neco jako:
    // sequence="LGW_20071112_E1_CAM1"
//    sequence = dataset->newSequence(vtapi->commons->getSequence());
    sequence = dataset->newSequence(dataset->sequence);
    sequence->next();

    // init VideoCapture
    String filename = dataset->baseLocation + "i-LIDS/Gatwick/bt/" + sequence->getName() + ".BT.avi";
    qDebug() << filename.c_str();
    videoCapture = VideoCapture(filename); // FIXME cteni ze souboru
    if (!videoCapture.isOpened()) {
        emit eventLabelChanged( QString("Toz brachu, nemas todle video :(") );
        t1 = t2 = 0;
    }
    else {
        qDebug() << "Opened...";
    }

    annotation = new Interval(*sequence, "annotations");
    // annotation->select->whereString("seqname", seqname); // toz todle je zbytecne, reguluje se v vtapi.conf :)
    // annotation->select->whereString("event", "OpposingFlow");
    if (annotation->next() != NULL) {
        emit eventLabelChanged( QString( annotation->getString("event").c_str() )  );

        t1 = annotation->getStartTime();
        t2 = annotation->getEndTime();

        emit eventPlusTime( spreadShot(t1, t2) );

        videoCapture.set(CV_CAP_PROP_POS_FRAMES, t1);
    }

    // tohle je tuna na vkladani, tak netreba next()...
    annTrack = new Interval(*sequence, "annotation_tracks");

    // start the timer
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    timer.start(50); // 1000/25 [ms] = 40

    // cyklus paralelniho zpracovani                                            // while (true)
    while (true) {
        // be doomed
        if(doom) break;


        // dela neco, kdyz je neco stisknute...
        if (nextClicked || previousClicked) {
            if (nextClicked) {
              nextClicked = false;
            }

            // uloz co prislo od toho/te na zidli
            QStringList tracks = text.split(",", QString::SkipEmptyParts);
            text = "";
            // kdyz je delka 0 nebo mam "", tak nic neukladam, jinak joo :)
            for (int i = 0; i < tracks.size(); ++i) {
                int track = atoi(tracks.at(i).toLocal8Bit().constData());

                annTrack->add(annotation->getSequenceName(), annotation->getStartTime(), annotation->getEndTime());
                annTrack->insert->keyString("event", annotation->getString("event"));
                annTrack->insert->keyInt("track", track);
                annTrack->insert->keyInt("xgtf_id", annotation->getInt("xgtf_id"));
                annTrack->addExecute();
            }
            
            // previous: SMALL HACK: pro přesun na předchozí pozici
            // POZOR!!! Nemuze to (previousClicked) byt pred predchozim vlozenim anotaci jinak budeme mit anotacni gulas.
            if (previousClicked) {
              previousClicked = false;
	      
              // To ani nezkousej - vic nez pres jednotku do zaporu Te nepustim..
              if (annotation->pos > 0) {
                annotation->pos -= 2;
              }
              else {
                annotation->pos = -1;
              }
            }


            // jestli se NEda pretocit v tomdle videu... ?
            if (annotation->next() != NULL) {
                emit eventLabelChanged( QString( annotation->getString("event").c_str() )  );

                t1 = annotation->getStartTime();
                t2 = annotation->getEndTime();
                
                emit eventPlusTime( spreadShot(t1, t2) );
                videoCapture.set(CV_CAP_PROP_POS_FRAMES, t1);
            }
            // to je konec :(
            else {
                emit eventLabelChanged( QString("Tadydadyda, to je konec!!!")  );
                /*
                if (sequence->next() == NULL) { // nebo sekvenci... ?
                    emit eventLabelChanged( QString("Tadydadyda, to je konec!!!")  );
                    break;
                }
                else {
                    emit eventLabelChanged( QString("Toz brachu, nech me chvilu...")  );
                    // nacti nove video
                    filename = dataset->baseLocation + "i-LIDS/Gatwick/bt/" + sequence->getName() + ".BT.avi";
                    qDebug() << filename.c_str();
                    videoCapture = VideoCapture(filename); // FIXME cteni ze souboru
                    if (!videoCapture.isOpened()) {
                        emit eventLabelChanged( QString("Toz brachu, nemas todle video :(") );
                        t1 = t2 = 0;
                    }
                    else {
                        emit eventLabelChanged( QString( annotation->getString("event").c_str() )  );

                        t1 = annotation->getStartTime();
                        t2 = annotation->getEndTime();
                        videoCapture.set(CV_CAP_PROP_POS_FRAMES, t1);
                    }
                }
                */
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
                resize(vc, mat, Size(), 1.0, 1.0, INTER_LINEAR); // toz mi ho trocha zvets
            }
            image = MatToQImage(mat);
            emit newFrame(image /* ... */);

            // cykli jak blbej, dokud to mezi zidlou a klavesnici neklikne na "Dalsi"
            if (videoCapture.get(CV_CAP_PROP_POS_AVI_RATIO) == 1 || videoCapture.get(CV_CAP_PROP_POS_FRAMES) > t2) {
                videoCapture.set(CV_CAP_PROP_POS_FRAMES, t1);
            }

        } // if (timerTick)

        // tohle je tu, aby to tak zase moc nezahrivalo procesor, akorat to obcas nevali :(
        msleep(10);
    } // while true ===

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