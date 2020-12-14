/*
   Copyright 2020 Simon Meaden

   Permission is hereby granted, free of charge, to any person obtaining a copy of this
   software and associated documentation files (the "Software"), to deal in the Software
   without restriction, including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software, and to permit
                                                                         persons to whom the Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all copies or
   substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
   INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
    PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
   OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
      SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QPainter>
#include <QMessageBox>

#include "simplecrypt.h"

/*!
   \file abstractlogindialog.h abstractlogindialog.cpp
   \class AbstractLoginDialog abstractlogindialog.h
   \brief The AbstractLoginDialog is the base class for login dialogs
   for ExTabWidget.

   If you create your own custom login dialog it needs to extand this class,
   otherwise you will not have the correct signals to handle login interaction
   with the ExTabWidget.

   The Simple and Standard login dialogs supplied with the ExTabWidget handle
   login checking.

   In the case of Simple login only a password needs to be supplied using
   ExTabWidget::setPassword(QString). This is for use in the VERY simple
   and rare case of having a single common password for the application login.

   In the Standard login, similarly, you need to use ExTabWidget::addPassword(QString, QString)
   to add username/password pairs to the ExTabWidget. Any number may be added, only one
   case of each username is allowed.

   If you need a more complex login dialog, quite likely, then you will need to handle
   password checking within your dialog, then emit either
   AbstractLoginDialog::logingCorrect(QString) on success, with an optional username,
   or AbstractLoginDialog::loginIncorrect() on failure.
*/
class AbstractLoginDialog : public QDialog
{
   Q_OBJECT

public:
   /*!
       \enum ::LoginType

      Defines which type of login dialog will be displayed.
   */
   enum LoginType
   {
      Simple,   //!< A simple password only dialog.
      Standard, //!< A more standard username/password dialog type
      Custom,   //!< A custom login dialog.
      None,     //!< No login dialog is set up.
   };

   enum LoginFailureType
   {
      NoFailure = 0,
      UnknownFailure = 1,
      PasswordShort = 2,
      NoPasswordSupplied = 4,
      UsernameShort = 8,
      NoUsernameSupplied = 16,
      NoSuchUser = 32,
      PasswordWrong = 64,
   };
   Q_DECLARE_FLAGS(LoginFailureTypes, LoginFailureType)
   Q_FLAG(LoginFailureTypes)

   //! Constructor for AbstractLoginDialog.
   //! Constructor for AbstractLoginDialog.
   explicit AbstractLoginDialog(QWidget* parent = nullptr,
                                quint64 key = 0x028eb11b3563ce52,
                                int minUsernameLength=6,
                                int minPasswordLength=6)
     : QDialog(parent)
     , m_key(key)
     , m_minUsernameLength(minUsernameLength)
     , m_minPasswordlength(minPasswordLength)
   {}

   //! Clears the username and password data  etc. from dialog.
   virtual void clearText() = 0;

   void setKey(quint64 key);

   //! Returns the minimum username length, default 6 characters.
   int minUsernameLength() const;
   //! Sets the minimum username length, default 6 characters.
   void setMinUsernameLength(int length);

   //! Returns the minimum password length, default 6 characters.
   int minPasswordLength() const;
   //! Sets the minimum password length, default 6 characters.
   void setMinPasswordLength(int length);

   static QString encrypt(const QString& password, quint64 key);
   static QString decrypt(const QString& password, quint64 key);


signals:

   /*!
      \fn ::loginCorrect()

      \brief This signal is should be emitted by your login dialog when the login
      data is correct.
   */
   void loginSucceeded(const QString& username = QString());

   /*!
      \fn ::loginFailed()

      \brief This signal is should be emitted by your login dialog when the login
       data is not correct.
   */
   void loginFailed(LoginFailureTypes types);

protected:
   quint64 m_key;
   int m_minUsernameLength, m_minPasswordlength;

   virtual void modifyStoredPassword(quint64 key) = 0;
};

class AbstractCustomLoginDialog : public AbstractLoginDialog
{
   Q_OBJECT

public:
   //! Constructor for AbstractCustomLoginDialog.
  explicit AbstractCustomLoginDialog(QWidget* parent = nullptr,
                                     quint64 key = 0x028eb11b3563ce52,
                                     int minUsernameLength=6,
                                     int minPasswordLength=6)
    : AbstractLoginDialog(parent, key, minUsernameLength, minPasswordLength)
  {}

   virtual void addUser(const QString& id, const QString& password) = 0;
   virtual void removeUser(const QString& username) = 0;
   virtual void clearUsers() = 0;
};

class StandardLoginDialog : public AbstractCustomLoginDialog
{
   Q_OBJECT
public:
   explicit StandardLoginDialog(quint64 key, int minUsernameLength, int minPasswordLength, QWidget* parent = nullptr);
   ~StandardLoginDialog();

   void addUser(const QString& username, const QString& password);
   void removeUser(const QString& username);
   void clearUsers();

   // AbstractLoginDialog interface
   void clearText() override;

   int minUsernameLength() const;
   void setMinUsernameLength(int minUsernameLength);

   int minPasswordLength() const;
   void setMinPasswordLength(int minPasswordLength);

   int exec() override;

private:
   QLineEdit* m_userEdit, *m_passEdit;
   QDialogButtonBox* m_buttons;
   QMap<QString, QString> m_passwords;
   int m_minUsernameLength = 6;
   int m_minPasswordLength = 6;

   void initGui(bool largeText);
   void enableOkBtn(const QString&);
   void checkPassword();
   void modifyStoredPassword(quint64 key) override;
};

class SimpleLoginDialog : public AbstractLoginDialog
{
   Q_OBJECT
public:
   explicit SimpleLoginDialog(quint64 key, QWidget* parent);
   ~SimpleLoginDialog();

   // AbstractLoginDialog interface
   void clearText() override;
   void setPassword(const QString& password);
   void clearUsers();

   int exec() override;

private:
   QLineEdit* m_passEdit;
   QString m_password;

   void initGui(bool largeText);
   void checkPassword();
   void modifyStoredPassword(quint64 key) override;
};


class LoginWidget : public QPushButton
{
   Q_OBJECT
public:
   explicit LoginWidget(QWidget* parent = nullptr, quint64 key = 0);

   bool isShowing();
   //! Returns true if the user is logged in, otherwise false.
   bool isLoggedIn();

   //   void logOut();

   bool isLoginEnabled();

   void showLogin(bool showLogin);

   void setCustomLoginDialog(AbstractLoginDialog* loginDlg);
   bool hasCustomLoginDialog();

   void setSimplePassword(const QString& password);
   void addUser(const QString& id, const QString& password);
   void removeUser(const QString& username);
   void clearUsers();

   AbstractLoginDialog::LoginType loginType() const;
   void setLoginType(const AbstractLoginDialog::LoginType& loginType);

   void setLoginText(const QString& loginText);
   void setLogoutText(const QString& logoutText);

   void setKey(quint64 key);

   int minUsernameLength() const;
   void setMinUsernameLength(int minUsernameLength);

   int minPasswordLength() const;
   void setMinPasswordLength(int minPasswordLength);


   static const QString LOGIN;
   static const QString LOGOUT;

signals:
   /// \cond DO_NOT_DOCUMENT
   void checkThePassword();
   /// \endcond

   void loginSucceeded(const QString& username = QString());
   void loginFailed(AbstractLoginDialog::LoginFailureTypes type);
   void loggedOut();

private:
  QString m_loginText, m_logoutText;
   QSize m_size;
   bool m_showLogin;
   AbstractLoginDialog::LoginType m_loginType;
   bool m_loggedIn;
   //   bool m_ignoreCase;
   AbstractLoginDialog* m_loginDlg, *m_customLoginDlg;
   int m_textLength;
   quint64 m_key = 0x028eb11b3563ce52;

  void loginWasSuccessful(const QString& username);
   void loginHasFailed(AbstractLoginDialog::LoginFailureTypes types);
   void loginBtnClicked();

};

Q_DECLARE_METATYPE(AbstractLoginDialog::LoginType)
Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractLoginDialog::LoginFailureTypes)

#endif // LOGINWIDGET_H
