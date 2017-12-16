#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui/QSystemTrayIcon>
#include <map>


namespace Ui {
class MainWindow;
}

class QTreeWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum class Feature
    {
        Device,
        Sensor,
        Fan,
        FanOutput,
        PowerMeter,
        LEDStripe
    };

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void addWidget(Feature feature, QWidget *widget, QString const& label);
    void setWidget(MainWindow::Feature feature, QWidget *widget);

    void saveSettings();
    void readSettings();

    void setVisible(bool visible) override;
    virtual bool event(QEvent *event) override;

signals:
    void signalResyncToDevice();
    void signalSettingsSave();
    void signalSettingsLoad();
    void signalSettingsClear();

    void currentChanged(int index);

public slots:
    void on_usbConectionChanged(bool connected);

protected:
    virtual void closeEvent(QCloseEvent *event) override;    

private slots:
    void on_actionQuit_triggered();
    void on_actionSync_triggered();

    void on_actionSave_Settings_triggered();
    void on_actionLoad_Settings_triggered();
    void on_actionClear_Settings_triggered();

    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void messageClicked();

    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void restoreSplitter();

private:
    void createTrayActions();
    void createTrayIcon();

    void setIcon(QIcon icon);

    void showTrayIconStatusMessage();
    void showTrayIconInfoMessage(const QString &msg, int timeout_msecs = 5000);

    void addTopLevelItemsForFeatures();
    void addFeatureItem(Feature feature, const QString &label);

    QIcon getIconByFeature(Feature feature, bool isTopLevel = false);

    Ui::MainWindow *ui;

    std::map<Feature, QTreeWidgetItem*> _itemFeature;

    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    std::map<QTreeWidgetItem*, int> _widgetToIndex;
};

#endif // MAINWINDOW_H
