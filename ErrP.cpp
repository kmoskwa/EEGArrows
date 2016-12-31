#include <scenario.h>
#include <QtGui>
#include <QGridLayout>
#include <QImage>
#include <QTime>
#include <math.h>
#include <Windows.h>

#include "screentimecode.h"
#include "EEGOpenVibe.h"
#include "ErrP.h"

class ErrPPrivate
{
public:
  ErrPPrivate(QObject *parent) : m_AutoMode(true), m_Direction(ERRP_NONE), m_ErrorPercentage(0)
  {
    m_tc = new ScreenTimecode(parent);
    m_openVibe = EEGOpenVibe::getInstance();
    bool ret = true;
    ret = QObject::connect(m_openVibe, SIGNAL(signalStimulationSent    (int, int) ),
                parent,         SLOT(  slotStimulationReceived(int, int) ));
    Q_ASSERT(ret);
  }
  ~ErrPPrivate()
  {
    delete m_tc;
   delete m_Timer;
  }
  enum ERRP_DIRECTION
  {
    ERRP_NONE,
    ERRP_LEFT,
    ERRP_RIGHT,
    ERRP_DONE,
  };
  ERRP_DIRECTION m_Direction;
  bool m_AutoMode;
  int m_ErrorPercentage;
  QPointer <QTimer> m_Timer;
  QTime m_StartTime;
  QImage m_FullArrowLeft;
  QImage m_FullArrowRight;

  QImage m_ArrowLeftGreen      ;
  QImage m_ArrowLeftGreenOnly  ;
  QImage m_ArrowLeftNormal     ;
  QImage m_ArrowLeftRed        ;
  QImage m_ArrowLeftRedOnly    ;
  QImage m_ArrowRightGreen     ;
  QImage m_ArrowRightGreenOnly ;
  QImage m_ArrowRightNormal    ;
  QImage m_ArrowRightRed       ;
  QImage m_ArrowRightRedOnly   ;
  QImage m_Blank               ;

  QLabel m_ArrowLeft;
  QLabel m_ArrowRight;
  QLabel m_Time;

  QPointer<ScreenTimecode> m_tc;
  QPointer<EEGOpenVibe> m_openVibe;

  int getRandomNo(int low, int high)
  {
    return qrand() % ((high + 1) - low) + low;
  }

  ERRP_DIRECTION getRandomDirection()
  {
    int rnd = getRandomNo(0, 1);
    if (rnd == 1) { return ERRP_LEFT;} else {return ERRP_RIGHT;}
  }

  bool getRandomError(bool isOK)
  { 
    if (false == isOK)
    {
      return false;
    }
    int rnd = getRandomNo(0, 100);
    if (rnd < m_ErrorPercentage)
    {
      return false;
    }
    return true;
  }

  void sendOpenVibeStimulation(int number, bool up, bool down = false)
  {
    if (true == up)
    {
      m_openVibe->slotSimulationReceived("openvibe-vrpn", number, 1);
    }
    if (true == down)
    {
      m_openVibe->slotSimulationReceived("openvibe-vrpn", number, 0);
    }
  }
  void startTimer(ErrP *mainClass, bool random = false, double maxtime = 2.0f, double minTime = 0.0f)
  {
    if (false == m_Timer.isNull())
    {
      delete m_Timer;
    }
    m_Timer = new QTimer();
    int interval = maxtime * 1000;
    if (true == random)
    {
      int minInterval = minTime * 1000;
      if (0 == minInterval)
      {
        minInterval = interval / 2;
      }
      interval = getRandomNo(minInterval, interval);
    }
    m_Timer->setInterval(interval);
    m_Timer->setSingleShot(true);
    bool ret = false;
    ret = QObject::connect(m_Timer, SIGNAL(timeout()), mainClass, SLOT(slotTimerTimeout()     ));
    Q_ASSERT(ret);
    m_Timer->start();
  }
  void stopTimer()
  {
    delete m_Timer;
    delete m_Timer;
  }

  void armLabel(int num)
  {
    QString label = QString("A%1").arg(num);
    m_tc->slotSetAuxLabel(label, Qt::red);
  }

  void stimLabel(int num)
  {
    QString label = QString("S%1").arg(num);
    m_tc->slotSetAuxLabel(label, Qt::green);
  }
};


ErrP::ErrP(QWidget *parent)
  : QWidget(parent)
{
  setWindowTitle(tr("ErrP"));
  p = new ErrPPrivate(this);
  QPointer<QGridLayout> grid = new QGridLayout();
  this->setLayout(grid);
  grid->setContentsMargins(0, 0, 0, 0);
  p->m_ArrowLeft.setText("Arrow Left");
  p->m_ArrowRight.setText("Arrow Left");
  //p->m_Time.setText("");
  QFont font = p->m_Time.font();
  font.setPointSize(60);
  font.setBold(true);
  p->m_Time.setFont(font);
  grid->addWidget(&p->m_Time , 0, 1, 1, 2, Qt::AlignCenter);
  grid->addWidget(&p->m_ArrowLeft , 1, 1);
  grid->addWidget(&p->m_ArrowRight, 1, 2);
  p->m_ArrowLeftGreen       .load(":/ErrP/Resources/vArrowLeftGreen.png"       );
  p->m_ArrowLeftGreenOnly   .load(":/ErrP/Resources/vArrowLeftGreenOnly.png"   );
  p->m_ArrowLeftNormal      .load(":/ErrP/Resources/vArrowLeftNormal.png"      );
  p->m_ArrowLeftRed         .load(":/ErrP/Resources/vArrowLeftRed.png"         );
  p->m_ArrowLeftRedOnly     .load(":/ErrP/Resources/vArrowLeftRedOnly.png"     );
  p->m_ArrowRightGreen      .load(":/ErrP/Resources/vArrowRightGreen.png"      );
  p->m_ArrowRightGreenOnly  .load(":/ErrP/Resources/vArrowRightGreenOnly.png"  );
  p->m_ArrowRightNormal     .load(":/ErrP/Resources/vArrowRightNormal.png"     );
  p->m_ArrowRightRed        .load(":/ErrP/Resources/vArrowRightRed.png"        );
  p->m_ArrowRightRedOnly    .load(":/ErrP/Resources/vArrowRightRedOnly.png"    );
  p->m_Blank                .load(":/ErrP/Resources/vBlank.png"                );

  p->m_ArrowLeft.setPixmap(QPixmap::fromImage(p->m_Blank));
  p->m_ArrowRight.setPixmap(QPixmap::fromImage(p->m_Blank));

  QDesktopWidget *desktop = QApplication::desktop();

  int screenWidth = desktop->width();
  int screenHeight = desktop->height();
  int W = this->width();
  int H = this->height();
  int x = 15;//(screenWidth - W) / 2;
  int y = 150;//(screenHeight - H) / 2;

  this->resize(W, H);
  this->move( x, y );
  this->installEventFilter(this);
  QTime timeRnd = QTime::currentTime();
  qsrand((uint)timeRnd.msec());
  if (true == p->m_AutoMode)
  {
    p->startTimer(this, false, 2.0f);
  }
}

ErrP::~ErrP()
{
  this->removeEventFilter(this);
  delete p;
}

void ErrP::slotScenarioActionTrigerred(bool checked)
{
  QObject *sender = QObject::sender();
  QPointer<QAction> action = qobject_cast<QAction *>(sender);
  if (false == action.isNull())
  {
    QString actionText = action->text();
    m_scenarioButton->setText(actionText);
    actionText = actionText.remove("S");
    int scenario = QVariant(actionText).toInt();
    scenario::getInstance()->setCurrentScenario(scenario);
    slotScenarioChanged(scenario);
  }
}

void ErrP::slotScenarioChanged(int scenario)
{
  //m_scenarioButton->setActive(1);
}

void ErrP::slotTimerTimeout(void)
{
  if (p->m_Direction == ErrPPrivate::ERRP_NONE)
  {
    ErrPPrivate::ERRP_DIRECTION dir = p->getRandomDirection();
    p->m_Direction = dir;
    if (dir == ErrPPrivate::ERRP_LEFT)
    {
      p->m_ArrowLeft.setPixmap(QPixmap::fromImage(p->m_ArrowLeftNormal));
    }
    else
    {
      p->m_ArrowRight.setPixmap(QPixmap::fromImage(p->m_ArrowRightNormal));
    }
    p->m_StartTime.start();
    p->startTimer(this);
    // STIMULATION 1 UP
    p->sendOpenVibeStimulation(1, true);
    p->armLabel(1);
  }
  else
  {
    p->m_Direction = ErrPPrivate::ERRP_NONE;
    p->m_Time.setText("");
    p->m_ArrowLeft.setPixmap(QPixmap::fromImage(p->m_Blank));
    p->m_ArrowRight.setPixmap(QPixmap::fromImage(p->m_Blank));
    if (true == p->m_AutoMode)
    {
      p->startTimer(this, true, 5.0f);
    }
    // STIMULATION 1 DOWN
    p->sendOpenVibeStimulation(1, false, true);
    p->armLabel(0);
  }
}

bool ErrP::eventFilter(QObject *obj, QEvent *event)
{
  if ( obj == this )
  {
    if (event->type()==QEvent::KeyPress) 
    {
      QKeyEvent* key = static_cast<QKeyEvent*>(event);
      quint32 scanKey = key->nativeScanCode();
      ErrPPrivate::ERRP_DIRECTION keyDirection = ErrPPrivate::ERRP_NONE;
      if (scanKey == 29)//VK_LCONTROL)
      {
        keyDirection = ErrPPrivate::ERRP_LEFT;
      }
      else if (scanKey == 285)
      {
        keyDirection = ErrPPrivate::ERRP_RIGHT;
      }
      if (keyDirection != ErrPPrivate::ERRP_NONE)
      {
        bool isOK = false;
        if (p->m_Direction == keyDirection)
        {
          isOK = true;
          // send stimulation 2 UP/DOWN
          isOK = p->getRandomError(isOK);
          if (false == isOK)
          {
            // send stimulation 4 UP/DOWN
            p->sendOpenVibeStimulation(4, true, true);
            p->stimLabel(4);
            if (keyDirection == ErrPPrivate::ERRP_LEFT)
            {
              keyDirection = ErrPPrivate::ERRP_RIGHT;
            }
            else if (keyDirection == ErrPPrivate::ERRP_RIGHT)
            {
              keyDirection = ErrPPrivate::ERRP_LEFT;
            }
          }
          else
          {
            p->sendOpenVibeStimulation(2, true, true);
            p->stimLabel(2);
          }
          }
        }
        else
        {
          // STIMULATION 3 UP/DOWN
          p->sendOpenVibeStimulation(3, true, true);
          p->stimLabel(3);
        }

        if (keyDirection == ErrPPrivate::ERRP_LEFT)
        {
          if (p->m_Direction == ErrPPrivate::ERRP_LEFT || p->m_Direction == ErrPPrivate::ERRP_RIGHT)
          {
            QString text;
            if (true == isOK)
            {
              p->m_ArrowLeft.setPixmap(QPixmap::fromImage(p->m_ArrowLeftGreen));
              text = QString("OK: %1 ms").arg(p->m_StartTime.elapsed());
            }
            else
            {
              p->m_ArrowLeft.setPixmap(QPixmap::fromImage(p->m_ArrowLeftRedOnly));
              text = QString("Wrong: %1 ms").arg(p->m_StartTime.elapsed());
            }
            p->m_Direction = ErrPPrivate::ERRP_DONE;
            p->m_Time.setText(text);
            p->startTimer(this);
          }
        }
        else if (keyDirection == ErrPPrivate::ERRP_RIGHT)
        {
          QString text;
          if (p->m_Direction == ErrPPrivate::ERRP_LEFT || p->m_Direction == ErrPPrivate::ERRP_RIGHT)
          {
            p->m_Time.setText(QString("Elapsed: %1 ms").arg(p->m_StartTime.elapsed()));
            if (true == isOK)
            {
              p->m_ArrowRight.setPixmap(QPixmap::fromImage(p->m_ArrowRightGreen));
              text = QString("OK: %1 ms").arg(p->m_StartTime.elapsed());
            }
            else
            {
              p->m_ArrowRight.setPixmap(QPixmap::fromImage(p->m_ArrowRightRedOnly));
              text = QString("Wrong: %1 ms").arg(p->m_StartTime.elapsed());
            }
            p->m_Direction = ErrPPrivate::ERRP_DONE;
            p->m_Time.setText(text);
            p->startTimer(this);
          }
        }

        /*
        */
      }
      else 
      {
        return QObject::eventFilter(obj, event);
      }
      return true;
    } 
    else 
    {
      return QObject::eventFilter(obj, event);
    }
    return false;
  }
  return false;
}


void ErrP::slotStimulationReceived(int button, int isOn)
{
  if (false == isOn)
  {
    switch (button)
    {
      case 0:
      {
        // disable auto mode
        p->m_AutoMode = false;
        p->sendOpenVibeStimulation(9, true, true);
       // p->stopTimer();
        p->stimLabel(9);
      }
      break;
      case 9:
      {
        // enable auto mode
        p->m_AutoMode = true;
        p->startTimer(this);
        p->sendOpenVibeStimulation(9, true, true);
        p->stimLabel(9);
      }
      break;
      case 1:
      {
      }
      break;
      case 2:
      {
      }
      break;
      case 3:
      {
      }
      break;
      case 4:
      {
      }
      break;
      case 5:
      {
        p->startTimer(this);
        p->sendOpenVibeStimulation(5, true, true);
        p->stimLabel(5);
      }
      break;
      case 6:
      {
        // 100% errors
        p->m_ErrorPercentage = 100;
        p->sendOpenVibeStimulation(6, true, true);
        p->stimLabel(6);
      }
      break;
      case 7:
      {
        // 20 % errors
        p->m_ErrorPercentage = 20;
        p->sendOpenVibeStimulation(7, true, true);
        p->stimLabel(7);
      }
      break;
      case 8:
      {
        // no errors
        p->sendOpenVibeStimulation(8, true, true);
        p->stimLabel(8);
        p->m_ErrorPercentage = 0;
      }
      break;
      default: 
      {
        // intentionally
      }
    }
    //p->armLabel(button);
    //p->m_openVibe->slotSimulationReceived("openvibe-vrpn", button - 1, 1);
    //return;
  }
  return;
}
