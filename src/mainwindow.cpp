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

#if defined(GOODSTYLE)
  text = goodValues();
#elif defined(BADPROPERTY)
  text = badValues();
#endif

  m_editor->setPlainText(text);
  QMap<int, BookmarkData*>* bookmarks = new QMap<int, BookmarkData*>();
  bookmarks->insert(1, new BookmarkData());
  bookmarks->insert(5, new BookmarkData("Test string"));
  bookmarks->insert(8, new BookmarkData());
  bookmarks->insert(12, new BookmarkData());
  bookmarks->insert(14, new BookmarkData());
  m_editor->setBookmarks(bookmarks);
  m_editor->setShowNewlineMarkers(true);

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

QString
MainWindow::goodValues()
{
  // clang-format off
  auto text = QStringLiteral(
    "QPushButton#evilButton:pressed {\n"
    "  background-color : rgb(224, 0, 0);\n"
    "  border-style: inset;\n"
    "}\n"
    "\n"

    "QMenu::icon:checked {\n"
    "  background: gray;\n"
    "  border: 1px inset gray;\n"
    "  position: absolute;\n"
    "  top: 1px;\n"
    "  right: 1px;\n"
    "  bottom: 1px;\n"
    "  left: 1px;\n"
    "}\n"
    "\n"

    "QTabBar::tab:selected, QTabBar::tab:hover {\n"
    "  background: qlineargradient( x1: 0, y1: 0, x2: 0, y2: 1,\n"
    "    stop: 0 #fafafa, stop: 0.4 #f4f4f4,\n"
    "    stop: 0.5 #e7e7e7, stop: 1.0 #fafafa);\n"
    "}\n"
    "\n"

    "alternate-background-color: red;\n"
    "alternate-background-color: #fafafa ;\n"
    "alternate-background-color: rgba(255, 0, 0, 75%);\n"
    "alternate-background-color: rgb(255, 0, 0); "
    "background-position : bottom center;\n" // Good Alignment
    "background-position: top left;\n"       // Good Alignment
    "background-position: top right;\n"      // Good Alignment
    "background-attachment: scroll;\n"
    "background-attachment: fixed;\n"
    "background: white url(:/images/ring.png);\n"
    "background-repeat: repeat-x;\n"
    "background-repeat: repeat-y;\n"
    "background-repeat: repeat;\n"
    "background-repeat: no-repeat;\n"
    "background-position: top left;\n"
    "background-position: bottom center;\n"
    "background-position: left;\n"
    "background-position: right;\n"
    "background-position: center;\n"
    "background-clip: margin;\n"
    "background-clip: border;\n"
    "background-clip: padding;\n"
    "background-clip: content;\n"
    "background-origin: margin;\n"
    "background-origin: border;\n"
    "background-origin: padding;\n"
    "background-origin: content;\n"
    "border: 1px solid white;\n"
    "border-color: red;\n"
    "border-color: red blue;\n"
    "border-color: red blue green;\n"
    "border-color: red blue green yellow;\n"
    "border-color: red #fafafa"
    "  rgba(255, 0, 0, 75%)"
    "  hsva(240, 255, 255, 75%);\n "
    "border-top-color: red;\n"
    "border-right-color: red;\n"
    "border-bottom-color: red;\n"
    "border-left-color: red;\n"
    "border-style: dashed;\n"
    "border-style: dot-dash;\n"
    "border-style: dot-dot-dash;\n"
    "border-style: dotted;\n"
    "border-style: double;\n"
    "border-style: groove;\n"
    "border-style: inset;\n"
    "border-style: outset;\n"
    "border-style: ridge;\n"
    "border-style: solid;\n"
    "border-style: none;\n"
    "border-left-style: solid;\n"
    "border-right-style: solid;\n"
    "border-bottom-style: solid;\n"
    "border-top-style: solid;\n"
    "border-image: url(:/images/frame.png) 4;\n"
    "border-image: url(:/res/background.jpg) 0 0 0 0 stretch stretch;\n"
    "border-top: 1px solid white;\n"
    "border-bottom: 1px solid white;\n"
    "border-left: 1px solid white;\n"
    "border-right: 1px solid white;\n"
    "border-radius: 4px;"
    "border-top-left-radius: 4px;"
    "border-bottom-left-radius: 4px;"
    "border-top-right-radius: 4px;"
    "border-bottom-right-radius: 4px;"
    "border-width: 4px;"
    "border-width: 4px 2px;"
    "border-width: 4px 2px 4px;"
    "border-width: 4px 2px 4px 2px;\n"
    "border-color: red blue green yellow;\n"
    "position:	relative;\n"
    "position:	absolute;\n"
    "left: 2px;\n"
    "right: 2px;\n"
    "top: 2px;\n"
    "bottom: 2px;\n"
    "button-layout: 2;\n"
    "color : #567dbc;\n"                 // good color value
    "color: #7db;\n"                     // good color value
    "color: blue;\n"                     // good property
    "color: red;\n"                      // good property value
    "color: #fafafa;\n"                  // good color value
    "color: #FAFAFA;\n"                  // good color value
    "color: rgb(0xfa, 0xf4, 0xf4);\n"    // good property name
    "color: rgba(255, 0, 0, 75%);\n"     // good property name
    "color: rgb(100%, 0%, 0%);\n"        // good property name
    "color: hsv(60, 100%, 100%);\n"      // good property name
    "color: hsva(240, 255, 255, 75%);\n" // good property name
    "color: hsl(60, 100%, 50%);\n"       // good property name
    "color: hsla(240, 255, 50%, 75%);\n" // good property name
    "dialogbuttonbox-buttons-have-icons: 0;\n"
    "dialogbuttonbox-buttons-have-icons: 1;\n"
    "file-icon: url(\":icons/add-dcolon\");\n" // good url
    "file-icon: url(:icons/add-dcolon);\n"     // good url
    "font: bold italic large \"Times New Roman\";\n"
    "font-family: \"Times New Roman\";\n"
    "font-size: xx-small;\n"
    "font-size: x-small;\n"
    "font-size: small;\n"
    "font-size: medium;\n"
    "font-size: large;\n"
    "font-size: x-large;\n"
    "font-size: xx-large;\n"
    "font-size: xxx-large;\n"
    "font-size: smaller;\n"
    "font-size: larger;\n"
    "font-size: 14px;\n"
    "font-size: 14pt;\n"
    "font-style: normal;\n"
    "font-style: italic;\n"
    "font-style: oblique;\n"
    "font-weight: normal;\n"
    "font-weight: bold;\n"
    "font-weight: 100;\n"
    "font-weight: 200;\n"
    "font-weight: 300;\n"
    "font-weight: 400;\n"
    "font-weight: 500;\n"
    "font-weight: 600;\n"
    "font-weight: 700;\n"
    "font-weight: 800;\n"
    "font-weight: 900;\n"
    "background: qlineargradient(\n" // good gradient
    "  x1: 0, y1: 0, x2: 0, y2: 1,\n"
    "  stop: 0 #fafafa, stop: 0.4 #f4f4f4,\n"
    "  stop: 0.5 #e7e7e7, stop: 1.0 #fafafa);\n"
    "}\n"
    "background: qconicalgradient(cx:0.5, cy:0.5, angle:30,\n"
    "  stop:0 white, stop:1 #00FF00);\n"
    "background: qradialgradient(cx:0, cy:0, radius: 1,\n"
    "  fx:0.5, fy:0.5, stop:0 white, stop:1 green);\n"
    "dialog-cancel-icon: url(cancel.png), url(grayed_cancel.png) disabled;\n"
    "gridline-color: red;\n"
    "height: 10px;\n"
    "icon-size: 10pt;\n"
    "image: url(:/images/spindown.png);\n"
    "image-position: top left;\n"
    "lineedit-password-character: 9679;\n"
    "lineedit-password-mask-delay: 1000;\n"
    "margin: 2px;\n"
    "margin: 2px 2px;\n"
    "margin: 2px 2px 2px;\n"
    "margin: 2px 2px 2px 2px;\n"
    "margin-left: 2px;\n"
    "margin-right: 2px;\n"
    "margin-top: 2px;\n"
    "margin-bottom: 2px;\n"
    "margin-bottom: 2px 2Px;\n"
    "max-height: 24px;\n"
    "min-height: 24px;\n"
    "min-width: 72px;\n"
    "messagebox-text-interaction-flags: 5;\n"
    "opacity: 223;\n"
    "outline-style: dotted;\n"
    "outline-style: solid;\n"
    "outline-style: double;\n"
    "outline-style: groove;\n"
    "outline-style: ridge;\n"
    "outline-style: inset;\n"
    "outline-style: outset;\n"
    "outline-style: none;\n"
    "outline-style: hidden;\n"
    "outline-style: dashed;\n"
    "outline-radius: 4px;\n"
    "outline-bottom-left-radius: 4px;\n"
    "outline-bottom-right-radius: 4px;\n"
    "outline-top-left-radius: 4px;\n"
    "outline-top-right-radius: 4px;\n"
    "outline-offset: 4px;\n"
    "outline-color: red;"
    "padding: 3px;\n"
    "padding: 3px 3px;\n"
    "padding: 3px 3px 3px;\n"
    "padding: 3px 3px 3px 3px;\n"
    "padding-top: 3px;\n"
    "padding-left: 3px;\n"
    "padding-bottom: 3px;\n"
    "padding-right: 3px;\n"
    "paint-alternating-row-colors-for-empty-area: true;\n"
    "paint-alternating-row-colors-for-empty-area: false;\n"
    "paint-alternating-row-colors-for-empty-area: 0;\n"
    "paint-alternating-row-colors-for-empty-area: 1;\n"
    "selection-background-color: darkblue;\n"
    "selection-color: white;\n"
    "show-decoration-selected: 1;\n"
    "spacing: 10px;\n"
    "color: palette(alternate-base);\n"
    "color: palette(base);\n"
    "color: palette(bright-text);\n"
    "color: palette(button);\n"
    "color: palette(button-text);\n"
    "color: palette(highlight);\n"
    "color: palette(highlighted-text);\n"
    "color: palette(light);\n"
    "color: palette(link);\n"
    "color: palette(link-visited);\n"
    "color: palette(mid);\n"
    "color: palette(midlight);\n"
    "color: palette(shadow);\n"
    "color: palette(text);\n"
    "color: palette(window);\n"
    "color: palette(window-text);\n")
    // clang-format on

    ;
  return text;
}

QString
MainWindow::badValues()
{
  // clang-format off
  QString text = ""
                 "border-color: blu #fafaf"
                 "  rgba(256, -1, 256, 175%)"
                 "  hsva(240, 255, 255, 75%);\n"
//                 "border-width: 4px 2px 4px 2px 3px;\n" // too many values
//                 "button-layout: 8;\n"
//                 "button-layout: 2.6;\n"
//                 "margin: 2px 2px 2px 2px 2px;\n"
    // clang-format on
    ;
  return text;
}

//"border-width: 4px 2px 4px 2px 3px;\n" // too many values
//"border-color: red "
//"  #fafafa rgba(255, 0, 0, 75 %) "
//"  hsva(240, 255, 255, 75%) "
//"  pink;\n" // too many values
//
//"messagebox-text-interaction-flags: -1;\n"
//"messagebox-text-interaction-flags: 32;\n"
//"opacity: -1;\n"
//"opacity: 256;\n"

//"file-icon: url();\n"
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
