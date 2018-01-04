#ifndef UBCONTROL_H
#define UBCONTROL_H

#include "QGCDockWidget.h"

class UBObject;

class QHash<class K, class V>;

namespace Ui {
class UBControl;
}

typedef QHash<quint8, UBObject*> OHash;

class UBControl : public QGCDockWidget
{
    Q_OBJECT

public:
    explicit UBControl(quint32 range, OHash* objs, OHash* lnks, const QString& title, QAction* action, QWidget *parent = 0);
    ~UBControl();

protected slots:
    void toggledEvent(bool checked);
    void textChangedEventRange();
    void textChangedEventNumber();
    void clickedEventConnect();
    void clickedEventDisconnect();

private:
    Ui::UBControl *ui;

    OHash* m_objs;
    OHash* m_lnks;
};

#endif // UBCONTROL_H
