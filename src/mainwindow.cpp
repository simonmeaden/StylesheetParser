/*
   Copyright 2020 Simon Meaden

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
#include "stylesheetedit/mainwindow.h"
#include "bookmarkarea.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QXmlStreamReader>
#include <QtDebug>

//#define GOODSTYLE
#define BADPROPERTY

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
{
  setCentralWidget(initGui());
  setGeometry(300, 100, 800, 800);
}

MainWindow::~MainWindow() {}

QWidget*
MainWindow::initGui()
{
  auto* f1 = new QFrame(this);
  auto* l1 = new QGridLayout;
  f1->setLayout(l1);

  m_lineBox = new QSpinBox(this);
  m_lineBox->setMinimum(1);

  auto gotoBtn = new QPushButton(tr("Go To Line"), this);
  connect(gotoBtn, &QPushButton::clicked, this, &MainWindow::gotoLine);

  m_toolBar = addToolBar(tr("Main"));
  m_statusBar = statusBar();
  m_lineLbl = new QLabel("--/--", this);
  m_colLbl = new QLabel("--", this);
  m_statusBar->addPermanentWidget(m_lineLbl);
  m_statusBar->addPermanentWidget(m_colLbl);

  m_editor = new StylesheetEdit(this);
  m_editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  l1->addWidget(m_editor, 0, 1);

  connect(
    m_editor, &StylesheetEdit::lineNumber, this, &MainWindow::setLineNumber);
  connect(
    m_editor, &StylesheetEdit::lineCount, this, &MainWindow::setLineCount);
  connect(
    m_editor, &StylesheetEdit::lineCount, m_lineBox, &QSpinBox::setMaximum);
  connect(m_editor, &StylesheetEdit::column, this, &MainWindow::setColumn);

  const QIcon leftIcon(":/icons/left");
  QAction* leftAct = new QAction(leftIcon, tr("&Left"), this);
  connect(leftAct, &QAction::triggered, m_editor, &StylesheetEdit::left);
  const QIcon rightIcon(":/icons/right");
  QAction* rightAct = new QAction(rightIcon, tr("&Right"), this);
  connect(rightAct, &QAction::triggered, m_editor, &StylesheetEdit::right);
  const QIcon upIcon(":/icons/up");
  QAction* upAct = new QAction(upIcon, tr("&Up"), this);
  connect(upAct, &QAction::triggered, m_editor, &StylesheetEdit::up);
  const QIcon downIcon(":/icons/down");
  QAction* downAct = new QAction(downIcon, tr("&Down"), this);
  connect(downAct, &QAction::triggered, m_editor, &StylesheetEdit::down);
  const QIcon startIcon(":/icons/start");
  QAction* startAct = new QAction(startIcon, tr("&Start"), this);
  connect(startAct, &QAction::triggered, m_editor, &StylesheetEdit::start);
  const QIcon endIcon(":/icons/end");
  QAction* endAct = new QAction(endIcon, tr("&End"), this);
  connect(endAct, &QAction::triggered, m_editor, &StylesheetEdit::end);
  const QIcon startLineIcon(":/icons/startline");
  QAction* startLineAct =
    new QAction(startLineIcon, tr("&Start of line"), this);
  connect(
    startLineAct, &QAction::triggered, m_editor, &StylesheetEdit::startOfLine);
  const QIcon endLineIcon(":/icons/endline");
  QAction* endLineAct = new QAction(endLineIcon, tr("&End of line"), this);
  connect(
    endLineAct, &QAction::triggered, m_editor, &StylesheetEdit::endOfLine);
  const QIcon goToIcon(":/icons/jump");
  QAction* goToAct = new QAction(goToIcon, tr("&Go To Line"), this);
  connect(goToAct,
          &QAction::triggered,
          m_editor,
          &StylesheetEdit::gotoBookmarkDialog);

  m_toolBar->addAction(startAct);
  m_toolBar->addAction(startLineAct);
  m_toolBar->addAction(leftAct);
  m_toolBar->addAction(upAct);
  m_toolBar->addAction(downAct);
  m_toolBar->addAction(rightAct);
  m_toolBar->addAction(endLineAct);
  m_toolBar->addAction(endAct);
  m_toolBar->addAction(goToAct);
  m_toolBar->addWidget(m_lineBox);
  m_toolBar->addWidget(gotoBtn);

  m_editor->addCustomWidget("StylesheetEdit", "QPlainTextEdit");
  QStringList list1;

  //  m_editor->addCustomWidgetPseudoStates("StylesheetEdit", states);
  //  m_editor->addCustomWidgetSubControls("StylesheetEdit", controls);
  list1 << "widget"
        << "subcontrol"
        << "pseudostate"
        << "subcontrolmarker"
        << "pseudostatemarker"
        << "property"
        << "propertymarker"
        << "value"
        << "startbrace"
        << "endbrace"
        << "bracematch"
        << "comment"
        << "bad";
  m_editor->addCustomWidgetProperties("StylesheetEdit", list1);
  m_editor->addCustomWidgetPropertyValues("StylesheetEdit", "widget", list1);
  list1.clear();
  m_editor->addCustomWidgetPropertyValues(
    "StylesheetEdit", "subcontrol", list1);
  m_editor->addCustomWidgetPropertyValues(
    "StylesheetEdit", "pseudostate", list1);
  m_editor->addCustomWidgetPropertyValues(
    "StylesheetEdit", "subcontrolmarker", list1);
  m_editor->addCustomWidgetPropertyValues(
    "StylesheetEdit", "pseudostatemarker", list1);
  m_editor->addCustomWidgetPropertyValues("StylesheetEdit", "property", list1);
  m_editor->addCustomWidgetPropertyValues(
    "StylesheetEdit", "propertymarker", list1);
  m_editor->addCustomWidgetPropertyValues("StylesheetEdit", "value", list1);
  m_editor->addCustomWidgetPropertyValues(
    "StylesheetEdit", "startbrace", list1);
  m_editor->addCustomWidgetPropertyValues("StylesheetEdit", "endbrace", list1);
  m_editor->addCustomWidgetPropertyValues("StylesheetEdit", "comment", list1);
  m_editor->addCustomWidgetPropertyValues("StylesheetEdit", "bad", list1);

  QString text;

  text =
#if defined(GOODSTYLE)
    "color: #567dbc;\n" // good color value GOOD
    "color: #7db;\n"    // good color value
    "color: blue;\n"    // good property
    //    "QTabBar :: tab : selected , QTabBar::tab:hover {\n"
    //    "background: qlineargradient(\n"
    //    "x1: 0, y1: 0, x2: 0, y2: 1,\n"
    //    "stop: 0 #fafafa, stop: 0.4 #f4f4f4,\n"
    //    "stop: 0.5 #e7e7e7, stop: 1.0 #fafafa);\n"
    //    "}\n"
    //    "QPushButton#evilButton {\n"
    //    "background-color: red;\n"
    //    "border-style: outset;\n"
    //    "border-width: 2px;\n"
    //    "border-radius: 10px;\n"
    //    "border-color: beige;\n"
    //    "font: bold 14px;\n"
    //    "min-width: 10em;\n"
    //    "padding: 6px;\n"
    //    "}\n"
    //    "QPushButton#evilButton:pressed {\n"
    //    "background-color: rgb(224, 0, 0);\n"
    //    "border-style: inset;\n"
    //    "}\n\n"
    //    // Good property.
    //    "color : red; \n"
    //    "background-color: blue;\n"
    //    "background-color: red\n" // good no end marker needed on last
    //    property "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
    //    stop: 0\n" "    #6ea1f1, stop: 1 #567dbc);\n" //
    ;
#elif defined(BADPROPERTY)

    //    "color: rd;\n"                         // bad property value FUZZY
    //    "color: #7dbc;\n"                      // bad color value
    //    "colr: blue;\n"                        // bad property name
    "file-icon: url(:icons/add-dcolon);\n" // good url
    "file-icon: url();\n"                  // no url value
    ;
  //    "background: \n" // bad gradient
  //    name
  //    "    qlinargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0\n"
  //    "    #6ea1f1, stop: 1 #567dbc);\n"
  //    "background: \n"               // bad color no #, wrong no of numerals
  //    "    qlineargradient(x1: ,\n"  // missing numerical value
  //    "                    y1: 0,\n" // good value
  //    "                    x: 0,\n"  // wrong name (x2)
  //    "                    y2: 1, \n"
  //    "                    y2: 1, \n"         // repeated name
  //    "                    stop: 0 6ea1f1,\n" // bad color no
  //    "                    stop: 1 #7dbc);\n" // wrong no of numerals (3 or 6)
  //    "background: \n" // Bad x1 - no value
  //    "    qlineargradient(x1: , y1: 0, x2: 0, y2: 1, stop: 0\n"
  //    "    #6ea1f1, stop: 1 #567dbc);\n"
  //    "background: \n" // Bad x1 no comma
  //    "    qlineargradient(x1: 0 y1: 0, x2: 0, y2: 1, stop: 0\n"
  //    "    #6ea1f1, stop: 1 #567dbc);\n"
  //    "background: \n" // Bad x1 no comma and no y2 value
  //    "    qlineargradient(x1: 0 y1: , x2: 0, y2: 1, stop: 0\n"
  //    "    #6ea1f1, stop: 1 #567dbc);\n"
  //    "background: \n" // Good
  //    "    qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0\n"
  //    "    #6ea1f1, stop: 1 #567dbc);\n"
  //    "border: green solid 1px\n"  // no end marker on non-final property
  //    "bckground-color: green; \n" // bad property name
  //;

#else

#endif

  // No Errors
  // Errors
  // Widget GOOD
  //         "QTabWidget::branch {color: red}\n"
  //         "QTabWidget::branch {color: red; background: green}\n"
  //         "QTabWidget::branch {color: red; background: green;}\n"
  //         "QTabWidget:active {color: red}\n"
  //         "QTabWidget:active {color: red; background: green}\n"
  //         "QTabWidget:active {color: red; background: green;}\n"
  //         "QTabWidget {color: red}\n"
  //         "QTabWidget {color: red; background: green}\n"
  //         "QTabWidget {color: red; background: green;}\n"
  // Widget Errors.
  //         "QTabWidget:branch {color: red}\n"                      // Bad
  //         marker "QTabWidget:branch {color: red; background: green}\n"   //
  //         Bad marker "QTabWidget:branch {color: red; background: green;}\n"
  //         // Bad marker "QTabWidget::active {color: red}\n" // Bad marker
  //         "QTabWidget::active {color: red; background: green}\n"  // Bad
  //         marker "QTabWidget::active {color: red; background: green;}\n" //
  //         Bad marker "QTabWidget::branh {}\n" // GOOD widget and BAD
  //         subcontrol
  //    "QTbWidget::branch {}\n" // BAD widget and GOOD subcontrol
  //    "QTbWidget::branh {}\n"  // BAD widget and BAD subcontrol
  //    "QTaWidget:actve {}\n"   // BAD widget and BAD pseudostate
  //    "QTabWidget:actve {}\n"  // BAD widget and BAD pseudostate
  //    "QTaWidget:active {}\n"  // BAD widget and GOOD pseudostate
  //    "QTabWidget:branch {\n" // Bad pseudostate - is subcontrol
  //    "  color: green\n"              // No end marker
  //    "  color: rd;\n"                // bad color value
  //    "  border: gren slid 1py\n"     // bad property values
  //    "  bckground-color: green; \n"  // bar property name
  //    "}\n"
  //    "QTabWidget::active {\n" // Bad subcontrol - is pseudostate
  //    "  color: green\n"              // No end marker
  //    "  color: rd;\n"                // bad color value
  //    "  border: gren slid 1py\n"     // bad property values
  //    "  bckground-color: green; \n"  // bar property name
  //    "}\n"
  // End of errors
  //    "QTabBar::close-button {\n"
  //    "    image: url(close.png);\n"
  //    "    subcontrol-position: left;\n"
  //    "}\n"
  //    "QTabBar::close-button:hover {\n"
  //    "    image: url(close-hover.png)\n"
  //    "}\n"
  //    "QTreeView::item:selected:active{\n"
  //    "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0
  //    #6ea1f1, stop: 1 #567dbc);\n"
  //    "}"
  //  ;
  //    "QTabWidget::branch {color: red; border: green solid 1px;\n" //
  //    MISSING END
  //                                                                 // BRACE
  //    "  /* comment1 \n"
  //    "*/ /*   \n"
  //    "comment2  */"
  //    "QTableWidget::branch {\n"
  //    "color: red;\n"
  //    "border: green solid 1px;\n"
  //    " background-color: blue; \n"
  //    "}\n"
  //    "\n";

  //         "/* comment1 */ /*   comment2  */ QTableWidget ::   branch {
  //         color: red; /* comment3 */ /*" " A multiline comment \n"
  //         "*/ }"
  //         "\n"

  m_editor->setPlainText(text);
  QMap<int, BookmarkData*>* bookmarks = new QMap<int, BookmarkData*>();
  bookmarks->insert(1, new BookmarkData());
  bookmarks->insert(5, new BookmarkData("Test string"));
  bookmarks->insert(8, new BookmarkData());
  bookmarks->insert(12, new BookmarkData());
  bookmarks->insert(14, new BookmarkData());
  m_editor->setBookmarks(bookmarks);
  m_editor->setShowNewlineMarkers(true);

  //    "color: red;\n"
  //    "color: red; background: green\n"
  //    "color: red; background: green;\n"
  //    "color: red; border: green solid 1px\n"
  //    "color: red\n"
  //    "border: green solid 1px;\n"
  //    "\n"
  //    "{}\n"

  //    "QTaWidget:active {}\n\n"             // BAD widget and GOOD
  ////    pseudostate    -    WORKING
  //    "QTabWidget::branh {}\n\n"            // BAD subcontrol -
  //    "QTbWidget::branch {}\n\n"            // BAD widget and GOOD
  //    subcontrol "QTbWidget::branh {}\n\n"             // BAD  widget and
  //    GOOD subcontrol "QTabWidget::branch {color: red; border: green solid
  //    1px; background-color: blue;" // BAD missing end brace, property end
  //    marker
  //    "QTabWidget::branch { color: red;
  //    border: green solid 1px; background-color: blue; }" // GOOD
  //    widget/subcontrol "QTabWidget";
  //  text = "QTaWidget:actve {}"; // BAD widget and pseudostate
  //  text = "QTaWidget:active {}"; // BAD widget and GOOD pseudostate
  //  text = "QTabWidget::branh {}"; // BAD subcontrol
  //  text = "QTbWidget::branch {}"; // BAD widget and GOOD subcontrol
  //  text = "QTbWidget::branh {}"; // BAD widget and GOOD subcontrol

  //    "\n\n"
  //    "color blu;" // BAD missing property marker        - WORKING
  //    "\n"
  //    "bordr: green slid 1px;"
  //    " \n\n"          // BAD property name, missing property end char -
  //    WORKING " shitwidget:active {}\n\n"             // BAD widget name -
  //    WORKING "background-color: blue;\n\n"        // GOOD property. -
  //    WORKING "color red; \n" "border: green solid 1px;\n" // missing first
  //    end property marker. text =  "color red; border: green solid 1px;"; //
  //    missing first end property marker.
  //  text =  "color: red border: green solid 1px;\n background-color: blue;";
  //  // missing first end property char. text =   "color: rd"; text = "color:
  //  red;"; text =  "color: red; background: green"; text =  "color: red;
  //  background: green;";
  //    text =  "  color rd; border: gren solid 1x;";
  //  text =  "color: red border: green solid 1px;\n background-color: blue;";
  //  // missing first end property char.

  //  text = "QTabWidget::branch {color: red; border: green solid 1px;
  //  background-color: blue;"; // missing end brace text = ; // BAD
  //  pseudostate

  //    text = "StylesheetEdit {\n"
  //           "  widget: blue liht yellow;\n"
  //           "subcntrol: yelow blue normal;\n"
  //           " subcntrolmarker: blue blck yellow;\n"
  //           "}";

  // Correct stuff
  //  text =   "color: red";
  //  text =   "color: red;";
  //  text =  "color: red; background: green";
  //  text =  "color: red; background: green;";
  //  text =  "color: red; border: green solid 1px;";

  //  text = "QTabWidget::branch {color: red}";
  //  text = "QTabWidget::branch {color: red; background: green}";
  //  text = "QTabWidget::branch {color: red; border: green solid 1px;
  //  background-color: blue;}"; text = "QTabWidget::branch {\ncolor: red;\n}\n"
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
  //  QString stylesheet =
  //    "StylesheetEdit {"
  //    "widget: green light white;"
  //    "}";

  //  m_editor->setStyleSheet(stylesheet);

  //  layout->addWidget(m_editor, 0, 0);

  return f1;
}

void
MainWindow::setColumn(int value)
{
  m_column = value;
  setLabels();
}

void
MainWindow::setLineNumber(int value)
{
  m_linenumber = value;
  setLabels();
}

void
MainWindow::setLineCount(int value)
{
  m_linecount = value;
  setLabels();
}

void
MainWindow::setLabels()
{
  QString line("Line : %1/%2");
  line = line.arg(m_linenumber).arg(m_linecount);
  QString col("Col : %1");
  col = col.arg(m_column);

  m_lineLbl->setText(line);
  m_colLbl->setText(col);
}

void
MainWindow::gotoLine()
{
  m_editor->goToLine(m_lineBox->value());
}
