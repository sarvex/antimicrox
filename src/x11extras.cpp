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

#include "common.h"

#include "globalvariables.h"
#include "messagehandler.h"

#include <unistd.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>

#include <QDebug>
#include <QFileInfo>
#include <QThreadStorage>


#include "x11extras.h"


static QThreadStorage<X11Extras*> displays;

X11Extras* X11Extras::_instance = nullptr;

X11Extras::X11Extras(QObject *parent) :
    QObject(parent),
    knownAliases()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    _display = XOpenDisplay(nullptr);
    populateKnownAliases();
}

/**
 * @brief Close display connection if one exists
 */
X11Extras::~X11Extras()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    freeDisplay();
}


template <typename T>
void freeWindow(T* window)
{
    if (window != nullptr)
    {
        XFree(window);
        window = nullptr;
    }
}


void X11Extras::freeDisplay()
{
    if (_display != nullptr)
    {
        XCloseDisplay(_display);
        _display = nullptr;
    }
}


X11Extras *X11Extras::getInstance()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    X11Extras *temp = nullptr;

    if (!displays.hasLocalData())
    {
        temp = new X11Extras();
        displays.setLocalData(temp);
    }
    else
    {
        temp = displays.localData();
    }

    return temp;
}

void X11Extras::deleteInstance()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (displays.hasLocalData())
    {
        X11Extras *temp = displays.localData();
        delete temp;
        displays.setLocalData(nullptr);
    }
}

/**
 * @brief Get display instance
 * @return Display struct
 */
Display* X11Extras::display()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return _display;
}

bool X11Extras::hasValidDisplay()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return (_display != nullptr);
}

/**
 * @brief CURRENTLY NOT USED
 */
void X11Extras::closeDisplay()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    freeDisplay();
}

/**
 * @brief Grab instance of active display.
 */
void X11Extras::syncDisplay()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    _display = XOpenDisplay(nullptr);
}

/**
 * @brief Grab instance of specified display. Useful for having the GUI
 *     on one display while generating events on another during ssh tunneling.
 * @param Valid display string that X can use
 */
void X11Extras::syncDisplay(QString displayString)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QByteArray tempByteArray = displayString.toLocal8Bit();
    _display = XOpenDisplay(tempByteArray.constData());
}

void X11Extras::setCustomDisplay(QString displayString)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    GlobalVariables::X11Extras::_customDisplayString = displayString;
}

/**
 * @brief Return root window for a given X display
 * @param Screen number. If no value is passed, uses screen 1.
 * @return XID of the window
 */
unsigned long X11Extras::appRootWindow(int screen)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return (screen == -1) ? XDefaultRootWindow(display()) : XRootWindowOfScreen(XScreenOfDisplay(display(), screen));
}

/**
 * @brief Get appropriate alias for a known KeySym string that might be blank
 *     or contain invalid characters when returned from X.
 * @param QString representation of a KeySym string
 * @return Alias string or a blank QString if no alias was found
 */
QString X11Extras::getDisplayString(QString xcodestring)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();

    if (knownAliases.contains(xcodestring))
    {
        temp = knownAliases.value(xcodestring);
    }

    return temp;
}

void X11Extras::populateKnownAliases()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    // These aliases are needed for xstrings that would
    // return empty space characters from XLookupString
    if (knownAliases.isEmpty())
    {
        knownAliases.insert("Escape", trUtf8("ESC"));
        knownAliases.insert("Tab", trUtf8("Tab"));
        knownAliases.insert("space", trUtf8("Space"));
        knownAliases.insert("Delete", trUtf8("DEL"));
        knownAliases.insert("Return", trUtf8("Return"));
        knownAliases.insert("KP_Enter", trUtf8("KP_Enter"));
        knownAliases.insert("BackSpace", trUtf8("Backspace"));
        knownAliases.insert("ISO_Level3_Shift", trUtf8("Alt Gr"));
    }
}

Window X11Extras::findParentClient(Window window)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Window parent = 0;
    Window root = 0;
    Window *children = nullptr;
    unsigned int num_children = 0;
    Window finalwindow = 0;
    Display *display = this->display();

    checkPropertyOnWin(windowIsViewable(display, window) && isWindowRelevant(display, window), window, parent, finalwindow, root, children, display, num_children);

    return finalwindow;
}

/**
 * @brief Check window and any parents for the window property "_NET_WM_PID"
 * @param Window XID for window of interest
 * @return PID of the application instance corresponding to the window
 */
int X11Extras::getApplicationPid(Window window)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Atom atom, actual_type;
    int actual_format = 0;
    unsigned long nitems = 0;
    unsigned long bytes_after = 0;
    unsigned char *prop = nullptr;
    int pid = 0;
    Window finalwindow = 0;
    Window parent = 0;
    Window root = 0;
    Window *children = nullptr;
    unsigned int num_children;
    Display *display = this->display();
    atom = XInternAtom(display, "_NET_WM_PID", True);

    checkPropertyOnWin(windowHasProperty(display, window, atom), window, parent, finalwindow, root, children, display, num_children);

    if (finalwindow)
    {
        int status = 0;

        status = XGetWindowProperty(display, finalwindow, atom, 0, 1024, false, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);

        if ((status == 0) && (prop != nullptr))
        {
            pid = prop[1] << 8;
            pid += prop[0];
            XFree(prop);
        }
    }

    return pid;
}


void X11Extras::checkPropertyOnWin(bool windowCorrected, Window& window, Window& parent, Window& finalwindow, Window& root, Window *children, Display *display, unsigned int& num_children)
{
    if (windowCorrected)
    {
        finalwindow = window;
    }
    else
    {
        bool quitTraversal = false;

        while (!quitTraversal)
        {
            children = nullptr;

            if (XQueryTree(display, window, &root, &parent, &children, &num_children))
            {
                if (children != nullptr)
                {
                    // must be a test for NULL
                    XFree(children);
                }

                if (parent)
                {
                    if (windowCorrected)
                    {
                        quitTraversal = true;
                        finalwindow = parent;
                    }
                    else if (parent == 0)
                    {
                        quitTraversal = true;
                    }
                    else if (parent == root)
                    {
                        quitTraversal = true;
                    }
                    else
                    {
                        window = parent;
                    }
                }
                else
                {
                    quitTraversal = true;
                }
            }
            else
            {
                quitTraversal = true;
            }
        }
    }
}

/**
 * @brief Find the application file location for a given PID
 * @param PID of window
 * @return File location of application
 */
QString X11Extras::getApplicationLocation(int pid)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString exepath = QString();

    if (pid > 0)
    {
        QString procString = QString("/proc/%1/exe").arg(pid);
        QFileInfo procFileInfo(procString);

        if (procFileInfo.exists())
        {
            char buf[1024];
            QByteArray tempByteArray = procString.toLocal8Bit();
            ssize_t len = readlink(tempByteArray.constData(), buf, sizeof(buf)-1);

            if (len != -1)
            {
                buf[len] = '\0';
            }

            if (len > 0)
            {
                QString temp = QString::fromUtf8(buf);

                if (!temp.isEmpty())
                {
                    exepath = temp;
                }
            }
        }
    }

    return exepath;
}

/**
 * @brief Find the proper client window within a hierarchy. This check is needed
 *     in some environments where the window that has been selected is actually
 *     a child to a transparent parent window which was the one that was
 *     actually grabbed
 * @param Top window to check
 * @return Client window XID or 0 if no appropriate window was found
 */
Window X11Extras::findClientWindow(Window window)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Window parent = 1;
    Window root = 0;
    Window *children = nullptr;
    unsigned int num_children = 0;
    Window finalwindow = 0;
    Display *display = this->display();

    if (windowIsViewable(display, window) &&
        isWindowRelevant(display, window))
    {
        finalwindow = window;
    }
    else
    {
        XQueryTree(display, window, &root, &parent, &children, &num_children);

        if (children != nullptr)
        {
            for (unsigned int i = 0; (i < num_children) && !finalwindow; i++)
            {
                if (windowIsViewable(display, children[i]) &&
                    isWindowRelevant(display, window))
                {
                    finalwindow = children[i];
                }
            }
        }

        if (!finalwindow && (children != nullptr))
        {
            for (unsigned int i = 0; (i < num_children) && !finalwindow; i++)
            {
                finalwindow = findClientWindow(children[i]);
            }
        }

        freeWindow(children);
    }

    return finalwindow;
}

bool X11Extras::windowHasProperty(Display *display, Window window, Atom atom)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;

    Atom actual_type;
    int actual_format = 0;
    unsigned long nitems = 0;
    unsigned long bytes_after = 0;
    unsigned char *prop = nullptr;
    int status = 0;
    status = XGetWindowProperty(display, window, atom, 0, 1024, false, AnyPropertyType,
                                &actual_type, &actual_format, &nitems, &bytes_after,
                                &prop);

    if ((status == Success) && (prop != nullptr))
    {
        result = true;
    }

    freeWindow(prop);

    return result;
}

bool X11Extras::windowIsViewable(Display *display, Window window)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;
    XWindowAttributes xwa;
    XGetWindowAttributes(display, window, &xwa);

    if ((xwa.c_class == InputOutput) && (xwa.map_state == IsViewable))
    {
        result = true;
    }

    return result;
}

/**
 * @brief Go through a window's properties and search for an Atom
 *     from a defined list. If an Atom is found in a window's properties,
 *     that window should be considered relevant and one that should be grabbed.
 * @param Display*
 * @param Window
 * @return If a window has a relevant Atom in its properties.
 */
bool X11Extras::isWindowRelevant(Display *display, Window window)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;

    QList<Atom> temp;
    temp.append(XInternAtom(display, "WM_STATE", True));
    temp.append(XInternAtom(display, "_NW_WM_STATE", True));
    temp.append(XInternAtom(display, "_NW_WM_NAME", True));

    QListIterator<Atom> iter(temp);

    while (iter.hasNext())
    {
        Atom current_atom = iter.next();

        if (windowHasProperty(display, window, current_atom))
        {
            iter.toBack();
            result = true;
        }
    }

    return result;
}

QString X11Extras::getWindowTitle(Window window)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();

    Atom atom, actual_type;
    int actual_format = 0;
    unsigned long nitems = 0;
    unsigned long bytes_after = 0;
    unsigned char *prop = nullptr;
    int status = 0;

#ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "WIN: 0x" << QString::number(window, 16);
#endif

    Display *display = this->display();
    Atom wm_name = XInternAtom(display, "WM_NAME", True);
    Atom net_wm_name = XInternAtom(display, "_NET_WM_NAME", True);
    atom = wm_name;

    QList<Atom> tempList;
    tempList.append(wm_name);
    tempList.append(net_wm_name);
    QListIterator<Atom> iter(tempList);

    while (iter.hasNext())
    {
        Atom temp_atom = iter.next();

        if (windowHasProperty(display, window, temp_atom))
        {
            iter.toBack();
            atom = temp_atom;
        }
    }

    status = XGetWindowProperty(display, window, atom, 0, 1024, false, AnyPropertyType,
                                &actual_type, &actual_format, &nitems, &bytes_after,
                                &prop);

    if ((status == Success) && (prop != nullptr))
    {
        char *tempprop = reinterpret_cast<char*>(prop);
        temp.append(QString::fromUtf8(tempprop));

        #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << temp;
        #endif
    }

    freeWindow(prop);

    return temp;
}

QString X11Extras::getWindowClass(Window window)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();

    Atom atom, actual_type;
    int actual_format = 0;
    unsigned long nitems = 0;
    unsigned long bytes_after = 0;
    unsigned char *prop = nullptr;
    int status = 0;

    Display *display = this->display();
    atom = XInternAtom(display, "WM_CLASS", True);
    status = XGetWindowProperty(display, window, atom, 0, 1024, false, AnyPropertyType,
                                &actual_type, &actual_format, &nitems, &bytes_after,
                                &prop);

    if ((status == Success) && (prop != nullptr))
    {
        #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << nitems;
        #endif

        char *null_char = strchr(reinterpret_cast<char*>(prop), '\0');
        if (((reinterpret_cast<char*>(prop)) + nitems - 1) > null_char)
        {
            *(null_char) = ' ';
        }

        char *tempprop = reinterpret_cast<char*>(prop);
        temp.append(QString::fromUtf8(tempprop));

        #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << temp;
        qDebug() << reinterpret_cast<char*>(prop);
        #endif
    }

    freeWindow(prop);

    return temp;
}


unsigned long X11Extras::getWindowInFocus()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    unsigned long result = 0;
    Window currentWindow = 0;
    int focusState = 0;
    Display *display = this->display();

    XGetInputFocus(display, &currentWindow, &focusState);

    if (currentWindow > 0)
    {
        result = static_cast<unsigned long>(currentWindow);
    }

    return result;
}

/**
 * @brief Get QString representation of currently utilized X display.
 * @return
 */
QString X11Extras::getXDisplayString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return GlobalVariables::X11Extras::_customDisplayString;
}

int X11Extras::getGroup1KeySym(int virtualkey)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Display *display = this->display();
    int temp = XKeysymToKeycode(display, static_cast<KeySym>(virtualkey));

    return static_cast<int>(XkbKeycodeToKeysym(display, static_cast<KeyCode>(temp), 0, 0));
}

void X11Extras::x11ResetMouseAccelerationChange(QString pointerName)
 {
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int xi_opcode, event, error;
    xi_opcode = event = error = 0;
    Display *display = this->display();
    bool result = XQueryExtension(display, "XInputExtension", &xi_opcode, &event, &error);

    if (!result)
    {
        Logger::LogInfo(trUtf8("xinput extension was not found. No mouse acceleration changes will occur."));
    }
    else
    {
        int ximajor = 2, ximinor = 0;

        if (XIQueryVersion(display, &ximajor, &ximinor) != Success)
        {
            Logger::LogInfo(trUtf8("xinput version must be at least 2.0. No mouse acceleration changes will occur."));
            result = false;
        }
    }

    if (result)
    {
        XIDeviceInfo *all_devices = nullptr;
        XIDeviceInfo *current_devices = nullptr;
        XIDeviceInfo *mouse_device = nullptr;

        int num_devices = 0;
        all_devices = XIQueryDevice(display, XIAllDevices, &num_devices);

        findVirtualPtr(num_devices, current_devices, mouse_device, all_devices, pointerName);

        if (mouse_device != nullptr)
        {
            XDevice *device = XOpenDevice(display, static_cast<XID>(mouse_device->deviceid));

            int num_feedbacks = 0;
            int feedback_id = -1;
            XFeedbackState *feedbacks = XGetFeedbackControl(display, device, &num_feedbacks);
            XFeedbackState *temp = feedbacks;

            checkFeedback(temp, num_feedbacks, feedback_id);

            XFree(feedbacks);
            feedbacks = temp = nullptr;

            if (feedback_id <= -1)
            {
                Logger::LogInfo(trUtf8("PtrFeedbackClass was not found for virtual pointer."
                                   "No change to mouse acceleration will occur for device with id=%1").arg(device->device_id));

                result = false;
            }
            else
            {
                Logger::LogInfo(trUtf8("Changing mouse acceleration for device with id=%1").arg(device->device_id));

                XPtrFeedbackControl	feedback;
                feedback.c_class = PtrFeedbackClass;
                feedback.length = sizeof(XPtrFeedbackControl);
                feedback.id = static_cast<XID>(feedback_id);
                feedback.threshold = 0;
                feedback.accelNum = 1;
                feedback.accelDenom = 1;

                XChangeFeedbackControl(display, device, DvAccelNum|DvAccelDenom|DvThreshold,
                           reinterpret_cast<XFeedbackControl*>(&feedback));

                XSync(display, false);
            }

            XCloseDevice(display, device);
        }

        if (all_devices != nullptr)
        {
            XIFreeDeviceInfo(all_devices);
        }
     }
 }

void X11Extras::x11ResetMouseAccelerationChange()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    x11ResetMouseAccelerationChange(GlobalVariables::X11Extras::mouseDeviceName);
}

struct X11Extras::ptrInformation X11Extras::getPointInformation(QString pointerName)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    struct ptrInformation tempInfo;

    int xi_opcode, event, error;
    xi_opcode = event = error = 0;
    Display *display = this->display();

    bool result = XQueryExtension(display, "XInputExtension", &xi_opcode, &event, &error);

    if (result)
    {
        int ximajor = 2, ximinor = 0;

        if (XIQueryVersion(display, &ximajor, &ximinor) != Success)
        {
            Logger::LogInfo(QObject::trUtf8("xinput version must be at least 2.0. No mouse acceleration changes will occur."));
            result = false;
        }
    }

    if (result)
    {
        XIDeviceInfo *all_devices = nullptr;
        XIDeviceInfo *current_devices = nullptr;
        XIDeviceInfo *mouse_device = nullptr;

        int num_devices = 0;
        all_devices = XIQueryDevice(display, XIAllDevices, &num_devices);

        findVirtualPtr(num_devices, current_devices, mouse_device, all_devices, pointerName);

        if (mouse_device != nullptr)
        {
            XDevice *device = XOpenDevice(display, static_cast<XID>(mouse_device->deviceid));

            int num_feedbacks = 0;
            int feedback_id = -1;
            XFeedbackState *feedbacks = XGetFeedbackControl(display, device, &num_feedbacks);
            XFeedbackState *temp = feedbacks;

            checkFeedback(temp, num_feedbacks, feedback_id);

            if (feedback_id <= -1)
            {
                result = false;
            }
            else
            {
                XPtrFeedbackState *tempPtrFeedback = reinterpret_cast<XPtrFeedbackState*>(temp);
                tempInfo.id = feedback_id;
                tempInfo.accelNum = tempPtrFeedback->accelNum;
                tempInfo.accelDenom = tempPtrFeedback->accelDenom;
                tempInfo.threshold = tempPtrFeedback->threshold;
            }

            XFree(feedbacks);
            feedbacks = temp = nullptr;
            XCloseDevice(display, device);
        }

        if (all_devices != nullptr)
        {
            XIFreeDeviceInfo(all_devices);
        }
    }

    return tempInfo;
}


void X11Extras::findVirtualPtr(int num_devices, XIDeviceInfo *current_devices, XIDeviceInfo *mouse_device, XIDeviceInfo *all_devices, QString pointerName)
{
    for (int i = 0; i < num_devices; i++)
    {
        current_devices = &all_devices[i];

        if ((current_devices->use == XISlavePointer) &&
            (QString::fromUtf8(current_devices->name) == pointerName))
        {
            mouse_device = current_devices;
        }
    }
}


void X11Extras::checkFeedback(XFeedbackState *temp, int& num_feedbacks, int& feedback_id)
{
    for (int i = 0; (i < num_feedbacks) && (feedback_id == -1); i++)
    {
        if (temp->c_class == PtrFeedbackClass)
        {
            feedback_id = static_cast<int>(temp->id);
        }

        if ((i + 1) < num_feedbacks)
        {
            temp = reinterpret_cast<XFeedbackState*>(reinterpret_cast<char*>(temp) + temp->length);
        }
    }
}

struct X11Extras::ptrInformation X11Extras::getPointInformation()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getPointInformation(GlobalVariables::X11Extras::mouseDeviceName);
}

QPoint X11Extras::getPos()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    XEvent mouseEvent;
    Window wid = DefaultRootWindow(display());
    XWindowAttributes xwAttr;

    XQueryPointer(display(), wid,
                  &mouseEvent.xbutton.root, &mouseEvent.xbutton.window,
                  &mouseEvent.xbutton.x_root, &mouseEvent.xbutton.y_root,
                  &mouseEvent.xbutton.x, &mouseEvent.xbutton.y,
                  &mouseEvent.xbutton.state);

    XGetWindowAttributes(display(), wid, &xwAttr);
    QPoint currentPoint(mouseEvent.xbutton.x_root, mouseEvent.xbutton.y_root);
    return currentPoint;
}

QHash<QString, QString> const& X11Extras::getKnownAliases() {

    return knownAliases;
}
