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
#include "stylesheetparser/mainwindow.h"
#include "bookmarkarea.h"

#include <QtDebug>

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
{
  setCentralWidget(initGui());
  setGeometry(400, 100, 500, 800);
}

MainWindow::~MainWindow() {}

QWidget* MainWindow::initGui()
{
  m_toolBar = addToolBar(tr("Main"));
  m_statusBar = statusBar();
  m_lineLbl = new QLabel("--/--", this);
  m_colLbl = new QLabel("--", this);
  m_statusBar->addPermanentWidget(m_lineLbl);
  m_statusBar->addPermanentWidget(m_colLbl);

  m_editor = new StylesheetEdit(this);
  m_editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  connect(m_editor, &StylesheetEdit::lineNumber, this, &MainWindow::setLineNumber);
  connect(m_editor, &StylesheetEdit::lineCount, this, &MainWindow::setLineCount);
  connect(m_editor, &StylesheetEdit::column, this, &MainWindow::setColumn);

  const QIcon leftIcon(":/icons/left");
  QAction* leftAct = new QAction(leftIcon, tr("&Left"), this);
  connect(leftAct, &QAction::triggered, m_editor, qOverload<bool>(&StylesheetEdit::left));
  const QIcon rightIcon(":/icons/right");
  QAction* rightAct = new QAction(rightIcon, tr("&Right"), this);
  connect(rightAct, &QAction::triggered, m_editor, qOverload<bool>(&StylesheetEdit::right));
  const QIcon upIcon(":/icons/up");
  QAction* upAct = new QAction(upIcon, tr("&Up"), this);
  connect(upAct, &QAction::triggered, m_editor, qOverload<bool>(&StylesheetEdit::up));
  const QIcon downIcon(":/icons/down");
  QAction* downAct = new QAction(downIcon, tr("&Down"), this);
  connect(downAct, &QAction::triggered, m_editor, qOverload<bool>(&StylesheetEdit::down));
  const QIcon startIcon(":/icons/start");
  QAction* startAct = new QAction(startIcon, tr("&Start"), this);
  connect(startAct, &QAction::triggered, m_editor, &StylesheetEdit::start);
  const QIcon endIcon(":/icons/end");
  QAction* endAct = new QAction(endIcon, tr("&End"), this);
  connect(endAct, &QAction::triggered, m_editor, &StylesheetEdit::end);
  const QIcon startLineIcon(":/icons/startline");
  QAction* startLineAct = new QAction(startLineIcon, tr("&Start of line"), this);
  connect(startLineAct, &QAction::triggered, m_editor, &StylesheetEdit::startOfLine);
  const QIcon endLineIcon(":/icons/endline");
  QAction* endLineAct = new QAction(endLineIcon, tr("&End of line"), this);
  connect(endLineAct, &QAction::triggered, m_editor, &StylesheetEdit::endOfLine);
  const QIcon goToIcon(":/icons/jump");
  QAction* goToAct = new QAction(goToIcon, tr("&Go To Line"), this);
  connect(goToAct, &QAction::triggered, m_editor, &StylesheetEdit::gotoBookmarkDialog);

  m_toolBar->addAction(startAct);
  m_toolBar->addAction(startLineAct);
  m_toolBar->addAction(leftAct);
  m_toolBar->addAction(upAct);
  m_toolBar->addAction(downAct);
  m_toolBar->addAction(rightAct);
  m_toolBar->addAction(endLineAct);
  m_toolBar->addAction(endAct);
  m_toolBar->addAction(goToAct);

  QString text;
  // Correct stuff
  //  text =   "color: red";
  //  text =   "color: red;";
  //  text =  "color: red; background: green";
  //  text =  "color: red; background: green;";
  //  text =  "color: red; border: green solid 1px;";

  //  text = "QTabWidget::branch {color: red}";
  //  text = "QTabWidget::branch {color: red; background: green}";
  //  text = "QTabWidget::branch {color: red; border: green solid 1px; background-color: blue;}";
  //  text = "QTabWidget::branch {\ncolor: red;\n}\n"
  //         "QWidget:active {\nborder: blue;}";
  //  text = "QTabWidget:active {\ncolor: red;\n"
  //         "/* An inline comment */\n}\n"
  //         "/* This is another\n comment */"
  //         "QWidget::branch {\ncolor; blue;\n background-color: red;\n}";

  // Test StylesheetEdit properties.
  //  text = "StylesheetEdit {\n"
  //         "widget: blue light yellow;\n"
  //         "subcontrol: yellow blue normal;\n"
  //         "subcontrolmarker: blue black yellow;\n"
  //         "pseudostate: red lightgrey light;\n"
  //         "pseudostatemarker: lightgrey red extrabold;\n"
  //         "property: darkgrey lightblue bold;\n"
  //         "propertymarker: lightblue medium;\n"
  //         "value: darkgrey pink demibold;\n"
  //         "startbrace: red;\n"
  //         "endbrace: yellow normal;\n"
  //         "bracematch: blue lightgreen bold;\n"
  //         "comment: blue yellow thin;\n"
  //         "bad: red blue extralight;\n"
  //         "}";

  // Errors
  text =
    "\n\n"
    "color red;\n"                        // BAD missing property marker        - WORKING
    " bordr: green slid 1px; \n"          // BAD property name, missing property end char - WORKING
    " shitwidget:active {}\n"             // BAD widget name                    - WORKING
    " background-color: blue;\n\n"        // GOOD property.                     - WORKING
    "color red; border: green solid 1px;" // missing first end property marker.
    "QTabWidget:actve {}\n\n"             // BAD widget and pseudostate         - WORKING
    "QTaWidget:active {}\n\n"             // BAD widget and GOOD pseudostate    - WORKING
    "QTabWidget::branh {}\n\n"            // BAD subcontrol                     - WORKING
    "QTbWidget::branch {}\n\n"            // BAD widget and GOOD subcontrol     - WORKING
    "QTbWidget::branh {}\n\n"             // BAD widget and GOOD subcontrol     - WORKING
    "QTabWidget::branch {color: red; border: green solid 1px; background-color: blue;" // BAD missing end brace
    ;
  //    text =  "color red; border: green solid 1px;"; // missing first end property marker.
  //  text =  "color: red border: green solid 1px;\n background-color: blue;"; // missing first end property char.
  //  text =   "color: rd";
  //  text =   "color: red;";
  //  text =  "color: red; background: green";
  //  text =  "color: red; background: green;";
  //    text =  "  color rd; border: gren solid 1x;";
  //  text =  "color: red border: green solid 1px;\n background-color: blue;"; // missing first end property char.

  //  text = "QTabWidget::branch {color: red; border: green solid 1px; background-color: blue;"; // missing end brace
  //  text = ; // BAD pseudostate
  //  text = "QTaWidget:actve {}"; // BAD widget and pseudostate
  //  text = "QTaWidget:active {}"; // BAD widget and GOOD pseudostate
  //  text = "QTabWidget::branh {}"; // BAD subcontrol
  //  text = "QTbWidget::branch {}"; // BAD widget and GOOD subcontrol
  //  text = "QTbWidget::branh {}"; // BAD widget and GOOD subcontrol

  //    text = "StylesheetEdit {\n"
  //           "  widget: blue liht yellow;\n"
  //           "subcntrol: yelow blue normal;\n"
  //           " subcntrolmarker: blue blck yellow;\n"
  //           "}";

  //  QString stylesheet =
  //    "StylesheetEdit {"
  //    "widget: green light white;"
  //    "}";

  //  m_editor->setStyleSheet(stylesheet);
  m_editor->setPlainText(text);

  QMap<int, BookmarkData*>* bookmarks = new QMap<int, BookmarkData*>();
  bookmarks->insert(1, new BookmarkData());
  bookmarks->insert(5, new BookmarkData("Test string"));
  bookmarks->insert(10, new BookmarkData());
  m_editor->setBookmarks(bookmarks);
  //  layout->addWidget(m_editor, 0, 0);

  bool hasntText = m_editor->hasBookmarkText(1);
  bool hasText = m_editor->hasBookmarkText(5);
  QString bookmarkText = m_editor->bookmarkText(5);
  int count = m_editor->bookmarkCount();

  if (!hasntText && hasText && bookmarkText == "Test string" && count == 3) {
    qWarning() << tr("Tested Good");

  } else {
    qWarning() << tr("Tested Bad");
  }

  m_editor->goToLine(8);

  return m_editor;
}

void MainWindow::setColumn(int value)
{
  m_column = value;
  setLabels();
}

void MainWindow::setLineNumber(int value)
{
  m_linenumber = value;
  setLabels();
}

void MainWindow::setLineCount(int value)
{
  m_linecount = value;
  setLabels();
}

void MainWindow::setLabels()
{
  QString line("Line : %1/%2");
  line = line.arg(m_linenumber).arg(m_linecount);
  QString col("Col : %1");
  col = col.arg(m_column);

  m_lineLbl->setText(line);
  m_colLbl->setText(col);
}
