#include "UBControl.h"
#include "UBObject.h"

#include "ui_UBControl.h"

UBControl::UBControl(quint32 range, OHash* objs, OHash* lnks, const QString& title, QAction* action, QWidget *parent) :
    QGCDockWidget(title, action, parent),
    ui(new Ui::UBControl),
    m_objs(objs),
    m_lnks(lnks)
{
    ui->setupUi(this);

    ui->rangeEdit->setValidator(new QIntValidator(1, 100000));
    ui->numberEdit->setValidator(new QIntValidator(1, 254));

    connect(ui->ns3Button, SIGNAL(toggled(bool)), this, SLOT(toggledEvent(bool)));
    connect(ui->rangeEdit, SIGNAL(textChanged(QString)), this, SLOT(textChangedEventRange()));
    connect(ui->numberEdit, SIGNAL(textChanged(QString)), this, SLOT(textChangedEventNumber()));
    connect(ui->connectButton, SIGNAL(clicked(bool)), this, SLOT(clickedEventConnect()));
    connect(ui->disconnectButton, SIGNAL(clicked(bool)), this, SLOT(clickedEventDisconnect()));

    if (range) {
        ui->rangeEdit->setText(QString::number(range));
    } else {
        ui->ns3Button->setChecked(true);
    }

    if (m_objs && m_objs->begin() != m_objs->end()) {
        ui->numberEdit->setText(QString::number(m_objs->begin().key()));
    }

//    resize(minimumSize());
    setFixedSize(minimumSize());
//    setParent(parent, Qt::Tool);
    if (parent) {
        move(parent->geometry().center() - rect().center());
    }
}

UBControl::~UBControl()
{
    delete ui;
}

void UBControl::toggledEvent(bool checked) {
    quint32 rng;
    if (checked) {
        rng = 0;
        ui->rangeEdit->setEnabled(false);
    } else {
        rng = ui->rangeEdit->text().toUInt();
        ui->rangeEdit->setEnabled(true);
    }

    if (!m_lnks) {
        return;
    }

    for (OHash::iterator it = m_lnks->begin(); it != m_lnks->end(); ++it) {
        QMetaObject::invokeMethod(it.value(), "setRange", Qt::QueuedConnection, Q_ARG(quint32, rng));
    }
}

void UBControl::textChangedEventRange() {
    quint32 rng = ui->rangeEdit->text().toUInt();
    if (!rng) {
        rng = 1;
        ui->rangeEdit->setText(QString::number(rng));
        ui->rangeEdit->selectAll();

        return;
    }

    if (!m_lnks) {
        return;
    }

    for (OHash::iterator it = m_lnks->begin(); it != m_lnks->end(); ++it) {
        QMetaObject::invokeMethod(it.value(), "setRange", Qt::QueuedConnection, Q_ARG(quint32, rng));
    }
}

void UBControl::textChangedEventNumber() {
    quint32 num = ui->numberEdit->text().toUInt();
    if (!num) {
        num = 1;
        ui->numberEdit->setText(QString::number(num));
        ui->numberEdit->selectAll();

        return;
    }

    if (!m_lnks || !m_objs) {
        return;
    }

    bool connect = false;
    bool disconnect = false;
    if (m_objs->value(num, nullptr)) {
        if (m_lnks->value(num, nullptr)) {
            disconnect = true;
        } else {
            connect = true;
        }
    }

    ui->connectButton->setEnabled(connect);
    ui->disconnectButton->setEnabled(disconnect);
}

void UBControl::clickedEventConnect() {
    if (!m_lnks || !m_objs) {
        return;
    }

    quint32 num = ui->numberEdit->text().toUInt();
    m_lnks->insert(num, m_objs->value(num));

    ui->connectButton->setEnabled(false);
    ui->disconnectButton->setEnabled(true);
}

void UBControl::clickedEventDisconnect() {
    if (!m_lnks) {
        return;
    }

    quint32 num = ui->numberEdit->text().toUInt();
    m_lnks->remove(num);

    ui->connectButton->setEnabled(true);
    ui->disconnectButton->setEnabled(false);
}
