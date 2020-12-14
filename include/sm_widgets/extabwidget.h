#ifndef _H
#define _H

#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QTabWidget>
#include <QTimer>
#include <QWidget>

#include "loginwidget.h"
#include "sm_widgets_global.h"

class ExTabWidgetPrivate;

/*!
   \class ExTabWidget
   \file extabwidget.h extabwidget.cpp
   \class ExTabWidget extabwidget.h

   \brief An extension of QTabWidget with the ability to add various things in
   the unused space to the right of the tabs.

   \since 5.7.0
   \license The MIT License
   \copyright © 2019 - 2020 Simon Meaden. All rights reserved.

   There are three things that can be added :
   -# A login button
   -# A digital clock
   -# A single line message box.

   More than one of the options can be added at the same time with the order
   from right to left being the login button, the clock and finally the message
   box, with the message box taking all available free space.

   Login Button
   ============

   The login button is displayed/hidden using the ExTabWidget::enableLogin(bool)
   method, with true displaying the button.

   There are two varieties of login as standard, a fairly normal login dialog
   with both username and a password edit boxes and a 'simple' login which has only
   a password edit box. Use the
   ExTabWidget::setLoginType(AbstractLoginDialog::Simple) or
   ExTabWidget::setLoginType(AbstractLoginDialog::Standard) to enable the
   appropriate type.

   Alternatively you can create your own login dialog by extending
   AbstractCustomloginDioalog and setting the login type to Custom using
   ExTabWidget::setLoginType(AbstractLoginDialog::Custom).

   Important Note:
   ---------------
   ___
   Passwords in both the Simple and Standard login types are stored in an
   encrypted format but, and this is an important but, while the encryption is
   probably fine for an in-house project where only casual observation will be
   encountered, you will probably want to use a much higher level of encryption on
   any project where an external (or internal I suppose) hacker might be
   encountered. For this I would suggest that a custom dialog with much stronger
   internal encryption be created.
   ___

   Custom dialogs are added using the setCustomLoginDialog(AbstractLoginDialog*)
   method. For information on what you need to create a custom dialog see the
   AbstractCustomLoginDialog documentation.

   Clicking on the button when the log out is displayed will cause a
   ExTabWidget::loggedOut() signal to be sent. ExTabWidget::loggedIn() returns
   false at this point. isLoginEnabled() returns true if the clock is displayed,
   otherwise false.

   By default the login message is "Log In" and the log out message is "Log Out". This
   can be modified using a language module as can all the strings as they use the
   tr() method. The login messages can also be modified using the setLoginText(QString)
   and setLogoutText(QString) methods if a more esoteric message is required.

   The simple login method requires that a password is set, only one allowed,
   using the ExTabWidget::setSimplePassword(QString) method. The standard login
   requires that one or more username/password pairs be set up using the
   ExTabWidget::addStandardPassword(QString username, QString password) method,
   once for each user. User names must be unique, adding a second entry with the
   same username will overwrite the first entry.

   ExTabWidget::clearPasswords() will remove all passwords from the list.
   Passwords and usernames for the simple and standard login dialogs are kept
   separately so it is possible to switch between the two methods if required.

   Clock
   =====
   The clock is displayed when the ExTabWidget::enableClock(bool) is called with
   the value of true. By default the digital clock is displayed without seconds.
   The display seconds use the ExTabWidget::showseconds(bool) method.

   The frame style can be modified using the setClockFrameStyle(QFrame::Shape) method,
   but this will be ignored if a custom stylesheet border is used.

   isClockEnabled() returns true if the clock is displayed, otherwise false.

   Messages
   ========
   The message box can be displayed using the ExTabWidget::enableMessages(bool)
   method with a value of true. Messages are sent using the
   ExTabWidget::setMessage(QString, uint) method with the optional uint value
   indicating a timeout in seconds before the message is erased.

   If messages are too long for the available space you can use the marquee
   option which causes the message to rotate along message box. To enable this
   use the ExTabWidget::enableMarquee(bool) with a value of true, adjusting the
   speed if necessary with the setMarqueeSpeed(qreal) method.

   The frame style can be modified using the setMessageFrameStyle(QFrame::Shape) method,
   but this will be ignored if a custom stylesheet border is used.

   isMessageEnabled() returns true if the message box is displayed, otherwise false.

   General
   =======
   Each can be drawn with or without a frame, using the
   ExTabWidget::showFrame(bool) method. By default a frame is shown.

   StyleSheets
   ===========

   Stylesheets can be set using the ExTabWidget { ... } format. Stylesheets for
   the clock, login and message sections can be individually set using
   ExTabWidget::login { ... }, ExTabWidget::clock { ... } and
   ExTabWidget::message { ... } formats. For Example:
   ```
     ExTabWidget {
       color: red;
       background: lightblue;
       border-width: 1px;
       border-style: solid;
       border-color: blue;
       border-radius: 4px;
     }
     ExTabWidget::clock {
       color: lightblue;
       background: red;
       border-width: 1px;
       border-style: solid;
       border-color: red;
       border-radius: 4px;
     }
     ExTabWidget::login {
       color: lightgreen;
       background: red;
       border-width: 1px;
     }
     ExTabWidget::login {
       color: lightred;
       border-style: solid;
       border-color: red;
       border-radius: 4px;
     }
     ExTabWidget::message {
       color: lightblue;
       background: pink;
       border-width: 1px;
       border-style: solid;
       border-color: red;
       border-radius: 4px;
     }
   ```
   The two login sections above will be combined, and when sections are repeated only
   the last will be utilised.

   \since 5.0
*/
class SM_WIDGETS_SHARED_EXPORT ExTabWidget : public QTabWidget
{

   Q_OBJECT

   /*!
      \property ExTabWidget::clock

      \brief This property describes whether the clock is displayed.

      By default, this property is false, and the clock is not displayed.

      @accessor showClock(), isClockEnabled()
   */ Q_PROPERTY(bool clock READ isClockEnabled WRITE enableClock DESIGNABLE true)

   /*!
      \property ::messages

      \brief This property describes whether the message box is displayed.

      By default, this property is false, and messages are not displayed.

                               @accessor showMessages(), isMessagesEnabled()
   */
   Q_PROPERTY(
      bool messages READ isMessagesEnabled WRITE enableMessages DESIGNABLE true)

   /*!
      \property ExTabWidget::login

      \brief This property describes whether the login button is displayed.

      By default, this property is false, and the login button is not displayed.

      @accessor isLoginEnabled()
   */
   Q_PROPERTY(bool login READ isLoginEnabled)

   /*!
      \property ::loginType

      \brief This property sets the login dialog type.

      By default this is None, and no login button is diaplayed.

      @accessor loginType(), setLoginType()
   */
   Q_PROPERTY(
      AbstractLoginDialog::LoginType loginType READ loginType WRITE setLoginType)

   /*!
      \property ::marquee

      \brief  This property describes whether the marquee is enabled.

      @accessor enableMarquee(bool), isMarqueeEnabled()
   */
   Q_PROPERTY(
      bool marquee READ isMarqueeEnabled WRITE enableMarquee DESIGNABLE true)

   Q_DECLARE_PRIVATE(ExTabWidget)

public:
   /*!
      \brief Constructs an ExTabWidget with parent parent with an optional custom
      login dialog.
   */
   explicit ExTabWidget(
      QWidget* parent = nullptr,
      AbstractLoginDialog::LoginType type = AbstractLoginDialog::None,
      AbstractLoginDialog* customDlg = nullptr,
      quint64 key = 0);

   /*!
      \brief Destroys the tabbed widget.
   */
   ~ExTabWidget() {}

   // ReImplemented from QTabWidget.
   //! Reimplemented from QTabWidget::addTab(QWidget*, const QString&)
   int addTab(QWidget* page, const QString& label);
   //! Reimplemented from QTabWidget::addTab(QWidget*, const QIcon&, const QString&)
   int addTab(QWidget* page, const QIcon& icon, const QString& label);
   //! Reimplemented from QTabWidget::clear()
   void clear();
   //! Reimplemented from QTabWidget::insertTab(int, QWidget*, const QString&)
   int insertTab(int index, QWidget* page, const QString& label);
   //! Reimplemented from QTabWidget::insertTab(int, QWidget,* const QIcon&, const QString&)
   int insertTab(int index,
                 QWidget* page,
                 const QIcon& icon,
                 const QString& label);
   //! Reimplemented from QTabWidget::removeTab()
   void removeTab(int index);
   //! Reimplemented from QTabWidget::setIconSize()
   void setIconSize(const QSize& size);
   //! Reimplemented from QTabWidget::setTabIcon()
   void setTabIcon(int index, const QIcon& icon);
   //! Reimplemented from QTabWidget::setTabShape()
   void setTabShape(QTabWidget::TabShape s);
   //! Reimplemented from QTabWidget::setTabText()
   void setTabText(int index, const QString& label);
   //! Reimplemented from QTabWidget::setTabVisible()
   void setTabVisible(int index, bool visible);

   //! Reimplemented from QTabWidget::styleSheet();
   QString styleSheet() const;
   //! Reimplemented from QTabWidget::setStyleSheet()
   void setStyleSheet(const QString& stylesheet);

   // Clock methods
   //! Returns true if the clock is enabled, otherwise false.
   bool isClockEnabled();
   //! Enables/disables the display of the digital clock
   //! Shows the clock if true, otherwise hides it.
   void enableClock(bool enableClock);
   //! Shows a frame around the displayed items if true.
   //!
   //! The show frame flag operates on all of the extensions, so either they all
   //!  have frames or none of them have frames.
   void setClockFrameStyle(QFrame::Shape style = QFrame::NoFrame);
   //! Set the show seconds flag.
   //!
   //! Show the optional show seconds flag. Displays seconds when true, otherwise
   //! the seconds are not displayed.
   void showSeconds(bool showSeconds);
   //   //! Returns the stylesheet for the clock widget.
   //   QString clockStyleSheet() const;
   //   //! Sets the stylesheet for the clock widget alone.
   //   void setClockStyleSheet(const QString& styleSheet);

   // Login methods.

   /*!
      \brief Enables/disables the display of the login button

      Shows the login button if showLogin is true, otherwise hides it.
   */
   void enableLogin(bool enableLogin);
   /*!
      \brief Sets the login type to one of ExTabWidget::LoginType values.

      This is one of the LoginType values. By default it is ExTabWidget::None,
      however it can be set to ExTabWidget::Simple or ExTabWidget::Standard for
      one of the internal types or ExTabWidget::Custom if the user supplies a
      custom login dialog.

      \sa loginType(), ExTabWidget::LoginType
   */
   void setLoginType(AbstractLoginDialog::LoginType type);
   /*!
      \brief Returns the login type.

      This is one of the LoginType values. By default it is ExTabWidget::None,
      however it can be set to ExTabWidget::Simple or ExTabWidget::Standard for
      one of the internal types or ExTabWidget::Custom if the user supplies a
      custom login dialog.

      \sa setLoginType(), ExTabWidget::LoginType
   */
   AbstractLoginDialog::LoginType loginType();

   //! Modify the login text, by default "Log In" or a translated version.
   void setLoginText(const QString& text);
   //! Modify the logout text, by default "Log Out" or a translated version.
   void setLogoutText(const QString& text);
   //! The password encryption is encrypted utilising a 64 bit integer key.
   //! This method will allow you to use a personal encryption key, rather
   //! than the default key which will be available to anyone who downloads
   //! this code.
   void setKey(quint64 key);

   //! Adds a new user to the standard login. If you have created a custom
   //! login dialog this should also call your dialogs addUser() method.
   void addUser(QString username, QString password);
   //! Remove a user from the standard login. If you have created a custom
   //! login dialog this should also call your dialogs removeUser() method.
   void removeUser(const QString& username);
   //! Remove all users from the standard login. If you have created a custom
   //! login dialog this should also call your dialogs clearUsers() method.
   void clearUsers();

   //! Returns true if the user is logged in, otherwise false.
   bool isLoggedIn();

   //! Returns true if the login is enabled, otherwise false.
   bool isLoginEnabled();
   /*!
      \brief Sets a custom logging dialog.

      Note: This only stores the custom dialog object, it does not ensure tha it
            is used. To use the custom dialog you also need to call
            setLoginType(LoginType) method to ExTabWidget::Custom.
   */
   void setCustomLoginDialog(AbstractLoginDialog* loginDlg);

   //! Sets the password for the simple login dialog.
   void setSimplePassword(QString password);
   /*!
      \brief Adds a username/password pair for the standard login dialog.

      Usernames must be unique, inserting a repeat username will overwrite the
      earlier version.
   */
   void addStandardPassword(QString username, QString password);
   //! Clears all password/usernames.
   void clearPasswords();

   //   QString loginStyleSheet() const;
   //   void setLoginStyleSheet(const QString& styleSheet);

   // Message methods
   //!Returns true if the message box is enabled, otherwise false.
   bool isMessagesEnabled();

   //! Sets the message text scrolling.
   //!
   //! Sets the text message to scrolling if the text is longer than the available
   //! space. If the text will fit inside the available space then this is
   //! ignored.
   bool isMarqueeEnabled();
   //! Enables/disables the message box, visible if the value is true.
   void enableMessages(bool enableMessages);
   /*!
      \brief Sets the message text, with an optional timeout in seconds.

      Sets the text message for the tab bar. This is only a single line message
      so "\n" characters are removed. There really is not enough room for a multi
      line MessageTabWidget within the tab bar.
   */
   void setMessage(QString message, uint timeout = 0);

   //! Sets the frame style of the message window.
   //!
   //! \see QFrame::Shape
   //! \see QFrame::setFrameStyle()
   void setMessageFrame(QFrame::Shape style = QFrame::NoFrame);

   /*!
      \brief Sets the message text scrolling.

      Sets the text message to scrolling if the text is longer than the available
      space. If the text will fit inside the available space then this is
      ignored.
   */
   void enableMarquee(bool enable);

   /*!
      \brief Sets the marquee speed in characters a second.

      The default is 10 characters a second. To slow it down enter a lower
       number of characters a second and to speed it up a higher number.
   */
   void setMarqueeSpeed(qreal charPerSecond);

   /*!
      \brief Removes the message from the display.
   */
   void clearMessage();

   //   QString messageStyleSheet() const;
   //   void setMessageStyleSheet(const QString& styleSheet);

signals:
   /*!
       \fn ::loginCorrect()

      \brief This signal is should be emitted by your login dialog when the login
       data is correct.
   */
   void loginSucceeded(const QString& username = QString());

   /*!
       \fn ::loginIncorrect()

      \brief This signal is should be emitted by your login dialog when the login
       data is not correct.
   */
   void loginFailed(AbstractLoginDialog::LoginFailureTypes types);

   /*!
       \fn ::loggedOut()

      \brief This signal is should be emitted when the user has logged out.
   */
   void loggedOut();

   /// \cond DO_NOT_DOCUMENT
   // This signal is inended for internal use and informs the ExTabWidget
   // whenever a tab is added, removed or modified.
   void statusChanged();
   /// \endcond DO_NOT_DOCUMENT

private:
   void statusHasChanged();

   /*!
      \brief Reimplements: QTabWidget::resizeEvent()
   */
   virtual void resizeEvent(QResizeEvent* evt);

private:
   ExTabWidgetPrivate* d_ptr;
};

#endif // _H
