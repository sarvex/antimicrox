/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <SDL2/SDL_joystick.h>

#include <QMainWindow>
#include <QMap>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QFileDialog>

class InputDevice;
class CommandLineUtility;
class AntiMicroSettings;
class QWidget;
class QTranslator;
class AutoProfileInfo;
class AutoProfileWatcher;
class QLocalServer;
class AboutDialog;
class QAction;
class QMenu;
class QShowEvent;
class QEvent;
class QCloseEvent;
class JoyTabWidget;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{

    Q_OBJECT
    
public:
    MainWindow(QMap<SDL_JoystickID, InputDevice*> *joysticks,
               CommandLineUtility *cmdutility,
               AntiMicroSettings *settings,
               bool graphical=true, QWidget *parent = nullptr);
    ~MainWindow();

    bool getGraphicalStatus();

    void setTranslator(QTranslator *translator);
    QTranslator* getTranslator() const;

    void setAppTranslator(QTranslator *translator);
    QTranslator* getAppTranslator() const;

    QMap<int, QList<QAction*> > const& getProfileActions();

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void changeEvent(QEvent *event);
    virtual void closeEvent(QCloseEvent *event);

    //bool eventFilter(QObject *obj, QEvent *event) override;

    void retranslateUi();
    void loadConfigFile(QString fileLocation, int joystickIndex=0); // MainConfiguration class
    void loadConfigFile(QString fileLocation, QString controllerID); // MainConfiguration class
    void unloadCurrentConfig(int joystickIndex=0); // MainConfiguration class
    void unloadCurrentConfig(QString controllerID); // MainConfiguration class
    void changeStartSetNumber(int startSetNumber, QString controllerID); // MainConfiguration class
    void changeStartSetNumber(int startSetNumber, int joystickIndex=0); // MainConfiguration class

signals:
    void joystickRefreshRequested();
    void readConfig(int index); // MainConfiguration class
    void mappingUpdated(QString mapping, InputDevice *device);


public slots:
    void fillButtons();
    void makeJoystickTabs();
    void alterConfigFromSettings(); // MainConfiguration class
    void fillButtons(InputDevice *joystick);
    void fillButtons(QMap<SDL_JoystickID, InputDevice*> *joysticks);
    void startJoystickRefresh();
    void hideWindow();
    void saveAppConfig(); // MainConfiguration class
    void loadAppConfig(bool forceRefresh=false); // MainConfiguration class
    void removeJoyTabs();
    void quitProgram();
    void changeWindowStatus();
    void refreshTabHelperThreads();
    void controllerMapOpening();
    void testMappingUpdateNow(int index, InputDevice *device);
    void removeJoyTab(SDL_JoystickID deviceID);
    void addJoyTab(InputDevice *device);
    void selectControllerJoyTab(QString GUID);
    void selectControllerJoyTab(int index);
    void handleInstanceDisconnect();


private slots:
    void refreshTrayIconMenu();
    void trayIconClickAction(QSystemTrayIcon::ActivationReason reason);
    void mainMenuChange(QMenu* tempMenu);
    void disableFlashActions();
    void enableFlashActions();

    void joystickTrayShow(QMenu* tempmenu);
    void singleTrayProfileMenuShow();
    void profileTrayActionTriggered(QAction *action, bool checked);

    void populateTrayIcon();
    void openAboutDialog();
    void openJoystickStatusWindow();
    void openKeyCheckerDialog();
    void openGitHubPage();
    void openIssuesPage();
    void openWikiPage();
    void openCalibration();
    void propogateNameDisplayStatus(JoyTabWidget* tabwidget, bool displayNames);
    void changeLanguage(QString language); // MainConfiguration class
    void openMainSettingsDialog();
    void showStickAssignmentDialog();
    void checkHideEmptyOption();

#ifdef Q_OS_WIN
    void checkKeyRepeatOptions();
    void restartAsElevated();
#endif

    void openGameControllerMappingWindow(bool openAsMain=false);
    void propogateMappingUpdate(QString mapping, InputDevice *device);
    void autoprofileLoad(AutoProfileInfo *info); // MainConfiguration class
    void checkAutoProfileWatcherTimer(); // MainConfiguration class
    void updateMenuOptions();

private:
    Ui::MainWindow *ui;

    QMap<SDL_JoystickID, InputDevice*> *m_joysticks;
    QMap<int, QList<QAction*> > profileActions;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    AboutDialog *aboutDialog;
    CommandLineUtility *m_cmdutility;
    AntiMicroSettings *m_settings;
    QTranslator *m_translator;
    QTranslator *m_appTranslator;
    AutoProfileWatcher *appWatcher;

    QAction *hideAction;
    QAction *restoreAction;
    QAction *closeAction;
    QAction *updateJoy;

    bool signalDisconnect;
    bool showTrayIcon;
    bool m_graphical;

};

#endif // MAINWINDOW_H
