#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "command_queue.h"
#include "command_event.h"
#include "types.h"
#include <QMessageBox>
#include <QtGui/QCloseEvent>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->treeWidget->setRootIsDecorated(false);
    ui->treeWidget->setIconSize(QSize(32, 32));

    addTopLevelItemsForFeatures();
    createTrayActions();
    createTrayIcon();

    setIcon(QIcon(":/pixmaps/gFanStalled.svg"));
    trayIcon->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addFeatureItem(Feature feature, QString const &label)
{
    QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(label));
    item->setIcon(0, getIconByFeature(feature, true));
    ui->treeWidget->addTopLevelItem(item);
    ui->treeWidget->insertTopLevelItem(1000+static_cast<int>(feature), item);
    _itemFeature.insert(std::pair<Feature, QTreeWidgetItem*>(feature, item));
}

QIcon MainWindow::getIconByFeature(MainWindow::Feature feature, bool isTopLevel)
{
    if (isTopLevel)
    {
        switch (feature)
        {

        default:
            return QIcon(":/pixmaps/gFanStalled.svg");
            break;
        }
    }

    switch (feature)
    {
    default:
        return QIcon(":/pixmaps/gFanNotStalled.svg");
        break;
    }

       return QIcon();
}

void MainWindow::addTopLevelItemsForFeatures()
{
    addFeatureItem(Feature::Fan, tr("Fans"));
    addFeatureItem(Feature::FanOutput, tr("Fan Outputs"));
    addFeatureItem(Feature::LEDStripe, tr("LED Stripes"));
    addFeatureItem(Feature::PowerMeter, tr("Power Meters"));
}

void MainWindow::addWidget(MainWindow::Feature feature, QWidget *widget, const QString &label)
{
    QTreeWidgetItem* parentItem = 0;

    auto it = _itemFeature.find(feature);
    if (it != _itemFeature.end())
    {
        parentItem = it->second;
    }

    int index = ui->stackedWidget->addWidget(widget);

    QTreeWidgetItem *item = new QTreeWidgetItem(parentItem, QStringList(label));
    item->setIcon(0, getIconByFeature(feature));

    if (parentItem == 0)
        ui->treeWidget->insertTopLevelItem(static_cast<int>(feature), item);
    else
        parentItem->setExpanded(true);

    _widgetToIndex.insert(std::pair<QTreeWidgetItem*, int>(item, index));
}

void MainWindow::setWidget(MainWindow::Feature feature, QWidget *widget)
{
    QTreeWidgetItem* item = 0;

    auto it = _itemFeature.find(feature);
    if (it != _itemFeature.end())
    {
        item = it->second;
    }

    if (!item)
        return;

    int index = ui->stackedWidget->addWidget(widget);

    _widgetToIndex.insert(std::pair<QTreeWidgetItem*, int>(item, index));
}

void MainWindow::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

    trayIcon->setToolTip(tr("USB LED Fan Controller"));

    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::messageClicked);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::iconActivated);
}

void MainWindow::createTrayActions()
{
    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, &QAction::triggered, this, &QWidget::hide);

    maximizeAction = new QAction(tr("Ma&ximize"), this);
    connect(maximizeAction, &QAction::triggered, this, &QWidget::showMaximized);

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void MainWindow::setIcon(QIcon icon)
{
    trayIcon->setIcon(icon);
    setWindowIcon(icon);    
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    qDebug() << __PRETTY_FUNCTION__ << " reason: " << reason;

    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
        showTrayIconStatusMessage();
        break;
    case QSystemTrayIcon::DoubleClick:
        setVisible(true);
        break;
    case QSystemTrayIcon::MiddleClick:
        showTrayIconStatusMessage();
        break;
    default:
        ;
    }
}

void MainWindow::showTrayIconStatusMessage()
{
    showTrayIconInfoMessage("not connected");
}

void MainWindow::showTrayIconInfoMessage(QString const &msg, int timeout_msecs)
{
    QSystemTrayIcon::MessageIcon msgIcon = QSystemTrayIcon::MessageIcon(QSystemTrayIcon::MessageIcon::Information);
    trayIcon->showMessage("ULFController", msg, msgIcon, timeout_msecs);
}

void MainWindow::messageClicked()
{
    QMessageBox::information(0, tr("Systray"),
                               tr("Sorry, I already gave what help I could.\n"
                                  "Maybe you should try asking a human?"));
}

void MainWindow::setVisible(bool visible)
{
    minimizeAction->setEnabled(visible);
    maximizeAction->setEnabled(!isMaximized());
    restoreAction->setEnabled(isMaximized() || !visible);
    QMainWindow::setVisible(visible);
}

void MainWindow::saveSettings()
{
    qDebug() << __PRETTY_FUNCTION__;
    QSettings settings("Anorak", "ULFControl");
    settings.setValue("MainWindow/geometry", saveGeometry());
    settings.setValue("MainWindow/windowState", saveState());
    settings.setValue("MainWindow/splitter", ui->splitter->saveGeometry());

    int c = 0;
    for(int size : ui->splitter->sizes())
    {
        settings.setValue(QString("MainWindow/splitter-size-%1").arg(c++), size);
    }
}

void MainWindow::readSettings()
{
    QSettings settings("Anorak", "ULFControl");
    restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
    restoreState(settings.value("MainWindow/windowState").toByteArray());
    //ui->splitter->restoreGeometry(settings.value("MainWindow/splitter").toByteArray());
    restoreSplitter();
}

void MainWindow::restoreSplitter()
{
    qDebug() << __PRETTY_FUNCTION__;
    QSettings settings("Anorak", "ULFControl");

    QList <int> sizes;
    for(int c = 0; c < ui->splitter->sizes().size(); c++)
    {
        int restoredSize = settings.value(QString("MainWindow/splitter-size-%1").arg(c), 200).toInt();
        sizes.append(restoredSize);
    }

    ui->splitter->setSizes(sizes);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();

#ifdef Q_OS_OSX
    if (!event->spontaneous() || !isVisible())
    {
        return;
    }
#endif
    if (trayIcon->isVisible())
    {
        QMessageBox::information(this, tr("Systray"),
                                 tr("The program will keep running in the "
                                    "system tray. To terminate the program, "
                                    "choose <b>Quit</b> in the context menu "
                                    "of the system tray entry."));
        hide();
        event->ignore();
    }

    //QMainWindow::closeEvent(event);
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == (QEvent::Type)CommandEvents::EventAllDataUpdated)
    {
        CommandEvent *myEvent = static_cast<CommandEvent *>(event);
        // custom event handling here

        qDebug() << " response event, channel: " << myEvent->getChannel();

        return true;
    }

    return QWidget::event(event);
}

void MainWindow::on_usbConectionChanged(bool connected)
{
    if (connected)
        statusBar()->showMessage(("connected"));
    else
        statusBar()->showMessage(("not connected"));
}

void MainWindow::on_actionQuit_triggered()
{
    QCoreApplication::quit();
}

void MainWindow::on_actionSync_triggered()
{
    emit signalResyncToDevice();
}

void MainWindow::on_actionSave_Settings_triggered()
{
    emit signalSettingsSave();
}

void MainWindow::on_actionLoad_Settings_triggered()
{
    emit signalSettingsLoad();
}

void MainWindow::on_actionClear_Settings_triggered()
{
    emit signalSettingsClear();
}

void MainWindow::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous);

    auto it = _widgetToIndex.find(current);

    if (it == _widgetToIndex.end())
        return;

    int index = it->second;
    ui->stackedWidget->setCurrentIndex(index);
    emit currentChanged(index);
}
