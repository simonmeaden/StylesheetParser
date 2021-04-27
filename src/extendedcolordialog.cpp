#include "stylesheetedit/extendedcolordialog.h"
#include "stylesheetedit/x11colors.h"

#include <QtDebug>

const QString ColorDropLabel::DISPLAYLABELSTYLE = "background-color: %1;";
const QString ExtendedColorDialog::HASHCOLOR = "#%1%2%3";
const QString ExtendedColorDialog::HASHACOLOR = "#%1%2%3%4";
const QString ExtendedColorDialog::RGBCOLOR = "rgb(%1, %2, %3)";
const QString ExtendedColorDialog::RGBACOLOR = "rgba(%1, %2, %3, %4%)";
const QString ExtendedColorDialog::HSLCOLOR = "hsl(%1, %2, %3)";
const QString ExtendedColorDialog::HSLACOLOR = "hsla(%1, %2, %3, %4%)";
const QString ExtendedColorDialog::HSVCOLOR = "hsv(%1, %2, %3)";
const QString ExtendedColorDialog::HSVACOLOR = "hsva(%1, %2, %3,%4%)";

ExtendedColorDialog::ExtendedColorDialog(QWidget* parent)
  : QDialog(parent)
{
  initGui();
}

ExtendedColorDialog::ExtendedColorDialog(const QColor& initialColor,
                                         QWidget* parent)
  : QDialog(parent)
  , m_color(initialColor)
{
  initGui();
}

void
ExtendedColorDialog::initGui()
{
  setWindowTitle(tr("Choose Color"));

  m_tabs = new QTabWidget(this);
  auto layout = new QHBoxLayout;
  layout->addWidget(m_tabs);
  setLayout(layout);

  m_colorDlg = new QColorDialog(m_color, this);
  m_colorDlg->setOptions(QColorDialog::ShowAlphaChannel |
                         QColorDialog::DontUseNativeDialog);
  m_tabs->addTab(m_colorDlg, "Color Dialog");
  connect(m_colorDlg,
          &QColorDialog::accepted,
          this,
          &ExtendedColorDialog::acceptColor);
  connect(
    m_colorDlg, &QColorDialog::rejected, this, &ExtendedColorDialog::reject);
  connect(
    m_colorDlg, &QColorDialog::finished, this, &ExtendedColorDialog::finished);
  connect(m_colorDlg,
          &QColorDialog::colorSelected,
          this,
          &ExtendedColorDialog::colorSelected);
  connect(m_colorDlg,
          &QColorDialog::currentColorChanged,
          this,
          &ExtendedColorDialog::currentColorChanged);

  auto svgColors = initSvgFrame();
  m_tabs->addTab(svgColors, "SVG Color Names");

  auto x11Colors1 = initX11ColorFrame1();
  m_tabs->addTab(x11Colors1, "X11 Colors 1");
  auto x11Colors2 = initX11ColorFrame2();
  m_tabs->addTab(x11Colors2, "X11 Colors 2");
  auto x11Mono = initX11MonoFrame();
  m_tabs->addTab(x11Mono, "X11 Mono Shades");

  m_tabs->setCurrentIndex(2);
}

QTableWidgetItem*
ExtendedColorDialog::getSvgItem(/*int row, int column,*/ const QString& back,
                                const QString& fore)
{
  auto item = new QTableWidgetItem(back);
  item->setBackground(QColor(back));
  item->setForeground(QColor(fore));
  return item;
}

QTableWidgetItem*
ExtendedColorDialog::getX11Color1Item(const QString& back, const QString& fore)
{
  auto item = new QTableWidgetItem(back);
  auto b = QColorConstants::X11::color(back);
  if (b.isValid())
    item->setBackground(b);
  else
    qDebug();
  auto f = QColorConstants::X11::color(fore);
  if (f.isValid())
    item->setForeground(f);
  else
    qDebug();
  return item;
}

QTableWidgetItem*
ExtendedColorDialog::getX11Color2Item(const QString& back, const QString& fore)
{
  auto item = new QTableWidgetItem(back);
  auto b = QColorConstants::X11::color(back);
  if (b.isValid())
    item->setBackground(b);
  else
    qDebug();
  auto f = QColorConstants::X11::color(fore);
  if (f.isValid())
    item->setForeground(f);
  else
    qDebug();
  return item;
}

QTableWidgetItem*
ExtendedColorDialog::getX11MonoItem(const QString& back, const QString& fore)
{
  auto item = new QTableWidgetItem(back);
  auto b = QColorConstants::X11::color(back);
  if (b.isValid())
    item->setBackground(b);
  else
    qDebug();
  auto f = QColorConstants::X11::color(fore);
  if (f.isValid())
    item->setForeground(f);
  else
    qDebug();
  return item;
}

void
ExtendedColorDialog::selectSvgColor(int row, int column)
{
  auto item = m_svgTable->item(row, column);
  if (item) {
    m_color = item->background().color();
    m_name = item->text();
//    m_svgSelectionLbl->setPrimaryColor(QColorConstants::Svg::color(m_name));
    emit colorSelected(m_color);
  }
}

void
ExtendedColorDialog::selectX11_1Color(int row, int column)
{
  auto item = m_x11ColorTbl1->item(row, column);
  if (item) {
    m_color = item->background().color();
    m_name = item->text();
//    m_col1SelectionLbl->setPrimaryColor(QColorConstants::X11::color(m_name));
    emit colorSelected(m_color);
  }
}

void
ExtendedColorDialog::selectX11_2Color(int row, int column)
{
  auto item = m_x11ColorTbl2->item(row, column);
  if (item) {
    m_color = item->background().color();
    m_name = item->text();
//    m_col2SelectionLbl->setPrimaryColor(QColorConstants::X11::color(m_name));
    emit colorSelected(m_color);
  }
}

void
ExtendedColorDialog::selectX11_Mono(int row, int column)
{
  auto item = m_x11MonoTbl->item(row, column);
  if (item) {
    m_color = item->background().color();
    m_name = item->text();
//    m_monoSelectionLbl->setPrimaryColor(QColorConstants::X11::color(m_name));
    emit colorSelected(m_color);
  }
}

QDialogButtonBox*
ExtendedColorDialog::createBtnBox()
{
  auto btnbox =
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(btnbox,
          &QDialogButtonBox::accepted,
          this,
          &ExtendedColorDialog::acceptStandardColor);
  connect(btnbox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  return btnbox;
}

ColorDropLabel *ExtendedColorDialog::createColorDisplay()
{
  auto lbl = new ColorDropLabel(this);
  lbl->setStyleSheet("border: 2px darkslategray solid; border-radius: 25px;");
  lbl->setPrimaryColor(QColor("white"));
  lbl->setMinimumSize(QSize(300, 90));
  lbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  return lbl;
}

QFrame*
ExtendedColorDialog::initSvgFrame()
{
  auto frame = new QFrame(this);
  auto layout = new QGridLayout;
  frame->setLayout(layout);

  m_svgTable = new QTableWidget(20, 7, this);
  m_svgTable->setStyleSheet("QTableView { gridline-color: white; }");
  m_svgTable->horizontalHeader()->setVisible(false);
  m_svgTable->verticalHeader()->setVisible(false);
  m_svgTable->horizontalHeader()->setSectionResizeMode(
    QHeaderView::ResizeToContents);
  m_svgTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(m_svgTable, 0, 0);
  connect(m_svgTable,
          &QTableWidget::cellClicked,
          this,
          &ExtendedColorDialog::selectSvgColor);

  layout->addWidget(m_svgSelectionLbl = createColorDisplay(), 1, 0);

  layout->addWidget(createBtnBox(), 2, 0);

  int row = 0, column = 0;
  // blues
  m_svgTable->setItem(row++, column, getSvgItem("aliceblue"));
  m_svgTable->setItem(row++, column, getSvgItem("azure"));
  m_svgTable->setItem(row++, column, getSvgItem("lightcyan"));
  m_svgTable->setItem(row++, column, getSvgItem("paleturquoise"));
  m_svgTable->setItem(row++, column, getSvgItem("powderblue"));
  m_svgTable->setItem(row++, column, getSvgItem("lightblue"));
  m_svgTable->setItem(row++, column, getSvgItem("lightsteelblue"));
  m_svgTable->setItem(row++, column, getSvgItem("lightskyblue"));
  m_svgTable->setItem(row++, column, getSvgItem("skyblue"));
  m_svgTable->setItem(row++, column, getSvgItem("cornflowerblue"));
  m_svgTable->setItem(row++, column, getSvgItem("steelblue"));
  m_svgTable->setItem(row++, column, getSvgItem("dodgerblue"));
  m_svgTable->setItem(row++, column, getSvgItem("royalblue"));
  m_svgTable->setItem(row++, column, getSvgItem("deepskyblue"));
  m_svgTable->setItem(row++, column, getSvgItem("mediumslateblue"));
  m_svgTable->setItem(row++, column, getSvgItem("slateblue"));
  m_svgTable->setItem(row++, column, getSvgItem("indigo", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("darkslateblue", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("mediumblue", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("darkblue", "white"));

  row = 0;
  column++;
  m_svgTable->setItem(row++, column, getSvgItem("navy", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("blue", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("midnightblue", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("honeydew"));
  m_svgTable->setItem(row++, column, getSvgItem("limegreen"));
  m_svgTable->setItem(row++, column, getSvgItem("darkseagreen"));
  m_svgTable->setItem(row++, column, getSvgItem("palegreen"));
  m_svgTable->setItem(row++, column, getSvgItem("lightgreen"));
  m_svgTable->setItem(row++, column, getSvgItem("springgreen"));
  m_svgTable->setItem(row++, column, getSvgItem("lime"));
  m_svgTable->setItem(row++, column, getSvgItem("forestgreen", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("green", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("darkgreen", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("aquamarine"));
  m_svgTable->setItem(row++, column, getSvgItem("mediumaquamarine"));
  m_svgTable->setItem(row++, column, getSvgItem("mediumseagreen"));
  m_svgTable->setItem(row++, column, getSvgItem("mediumspringgreen"));
  m_svgTable->setItem(row++, column, getSvgItem("seagreen"));
  m_svgTable->setItem(row++, column, getSvgItem("greenyellow"));
  m_svgTable->setItem(row++, column, getSvgItem("lawngreen"));

  row = 0;
  column++;
  m_svgTable->setItem(row++, column, getSvgItem("chartreuse"));
  m_svgTable->setItem(row++, column, getSvgItem("yellowgreen"));
  m_svgTable->setItem(row++, column, getSvgItem("olivedrab", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("olive", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("darkolivegreen", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("cyan"));
  m_svgTable->setItem(row++, column, getSvgItem("aqua"));
  m_svgTable->setItem(row++, column, getSvgItem("turquoise"));
  m_svgTable->setItem(row++, column, getSvgItem("mediumturquoise"));
  m_svgTable->setItem(row++, column, getSvgItem("darkturquoise"));
  m_svgTable->setItem(row++, column, getSvgItem("lightseagreen"));
  m_svgTable->setItem(row++, column, getSvgItem("cadetblue"));
  m_svgTable->setItem(row++, column, getSvgItem("darkcyan"));
  m_svgTable->setItem(row++, column, getSvgItem("teal"));
  m_svgTable->setItem(row++, column, getSvgItem("seashell"));
  m_svgTable->setItem(row++, column, getSvgItem("lavenderblush"));
  m_svgTable->setItem(row++, column, getSvgItem("mistyrose"));
  m_svgTable->setItem(row++, column, getSvgItem("lightsalmon"));
  m_svgTable->setItem(row++, column, getSvgItem("darksalmon"));
  m_svgTable->setItem(row++, column, getSvgItem("salmon"));

  row = 0;
  column++;
  m_svgTable->setItem(row++, column, getSvgItem("tomato"));
  m_svgTable->setItem(row++, column, getSvgItem("red", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("pink"));
  m_svgTable->setItem(row++, column, getSvgItem("crimson", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("indianred", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("lavender"));
  m_svgTable->setItem(row++, column, getSvgItem("hotpink"));
  m_svgTable->setItem(row++, column, getSvgItem("lightpink"));
  m_svgTable->setItem(row++, column, getSvgItem("palevioletred"));
  m_svgTable->setItem(row++, column, getSvgItem("fuchsia"));
  m_svgTable->setItem(row++, column, getSvgItem("magenta"));
  m_svgTable->setItem(row++, column, getSvgItem("deeppink"));
  m_svgTable->setItem(row++, column, getSvgItem("mediumvioletred"));
  m_svgTable->setItem(row++, column, getSvgItem("thistle"));
  m_svgTable->setItem(row++, column, getSvgItem("plum"));
  m_svgTable->setItem(row++, column, getSvgItem("violet"));
  m_svgTable->setItem(row++, column, getSvgItem("orchid"));
  m_svgTable->setItem(row++, column, getSvgItem("mediumorchid"));
  m_svgTable->setItem(row++, column, getSvgItem("darkmagenta", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("purple", "white"));

  row = 0;
  column++;
  m_svgTable->setItem(row++, column, getSvgItem("mediumpurple"));
  m_svgTable->setItem(row++, column, getSvgItem("darkorchid", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("darkviolet", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("blueviolet", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("ivory"));
  m_svgTable->setItem(row++, column, getSvgItem("beige"));
  m_svgTable->setItem(row++, column, getSvgItem("lightyellow"));
  m_svgTable->setItem(row++, column, getSvgItem("linen"));
  m_svgTable->setItem(row++, column, getSvgItem("palegoldenrod"));
  m_svgTable->setItem(row++, column, getSvgItem("cornsilk"));
  m_svgTable->setItem(row++, column, getSvgItem("floralwhite"));
  m_svgTable->setItem(row++, column, getSvgItem("lemonchiffon"));
  m_svgTable->setItem(row++, column, getSvgItem("lightgoldenrodyellow"));
  m_svgTable->setItem(row++, column, getSvgItem("moccasin"));
  m_svgTable->setItem(row++, column, getSvgItem("wheat"));
  m_svgTable->setItem(row++, column, getSvgItem("khaki"));
  m_svgTable->setItem(row++, column, getSvgItem("darkkhaki"));
  m_svgTable->setItem(row++, column, getSvgItem("gold"));
  m_svgTable->setItem(row++, column, getSvgItem("goldenrod"));
  m_svgTable->setItem(row++, column, getSvgItem("darkgoldenrod"));

  row = 0;
  column++;
  m_svgTable->setItem(row++, column, getSvgItem("yellow"));
  m_svgTable->setItem(row++, column, getSvgItem("orange"));
  m_svgTable->setItem(row++, column, getSvgItem("darkorange"));
  m_svgTable->setItem(row++, column, getSvgItem("orangered", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("oldlace"));
  m_svgTable->setItem(row++, column, getSvgItem("antiquewhite"));
  m_svgTable->setItem(row++, column, getSvgItem("papayawhip"));
  m_svgTable->setItem(row++, column, getSvgItem("peachpuff"));
  m_svgTable->setItem(row++, column, getSvgItem("bisque"));
  m_svgTable->setItem(row++, column, getSvgItem("navajowhite"));
  m_svgTable->setItem(row++, column, getSvgItem("blanchedalmond"));
  m_svgTable->setItem(row++, column, getSvgItem("burlywood"));
  m_svgTable->setItem(row++, column, getSvgItem("tan"));
  m_svgTable->setItem(row++, column, getSvgItem("sandybrown"));
  m_svgTable->setItem(row++, column, getSvgItem("peru", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("sienna", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("chocolate", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("saddlebrown", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("coral"));
  m_svgTable->setItem(row++, column, getSvgItem("lightcoral"));

  row = 0;
  column++;
  m_svgTable->setItem(row++, column, getSvgItem("rosybrown"));
  m_svgTable->setItem(row++, column, getSvgItem("brown", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("firebrick", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("darkred", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("maroon", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("snow"));
  m_svgTable->setItem(row++, column, getSvgItem("white"));
  m_svgTable->setItem(row++, column, getSvgItem("whitesmoke"));
  m_svgTable->setItem(row++, column, getSvgItem("mintcream"));
  m_svgTable->setItem(row++, column, getSvgItem("ghostwhite"));
  m_svgTable->setItem(row++, column, getSvgItem("gainsboro"));
  m_svgTable->setItem(row++, column, getSvgItem("lightgray"));
  m_svgTable->setItem(row++, column, getSvgItem("silver"));
  m_svgTable->setItem(row++, column, getSvgItem("darkgray"));
  m_svgTable->setItem(row++, column, getSvgItem("lightslategray", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("slategray", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("darkslategray", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("gray", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("dimgray", "white"));
  m_svgTable->setItem(row++, column, getSvgItem("black", "white"));

  return frame;
}

QFrame*
ExtendedColorDialog::initX11ColorFrame1()
{
  auto frame = new QFrame(this);
  auto layout = new QGridLayout;
  frame->setLayout(layout);

  m_x11ColorTbl1 = new QTableWidget(17, 6, this);
  m_x11ColorTbl1->setStyleSheet("QTableView { gridline-color: white; }");
  m_x11ColorTbl1->horizontalHeader()->setVisible(false);
  m_x11ColorTbl1->verticalHeader()->setVisible(false);
  m_x11ColorTbl1->horizontalHeader()->setSectionResizeMode(
    QHeaderView::ResizeToContents);
  layout->addWidget(m_x11ColorTbl1, 0, 0);
  connect(m_x11ColorTbl1,
          &QTableWidget::cellClicked,
          this,
          &ExtendedColorDialog::selectX11_1Color);

  layout->addWidget(m_col1SelectionLbl = createColorDisplay(), 1, 0);
  layout->addWidget(createBtnBox(), 2, 0);

  int row = 0, column = 0;
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("mint cream"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("alice blue"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("azure"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("azure2"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("azure3"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("azure4"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("light cyan"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("pale turquoise"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("PaleTurquoise1"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("PaleTurquoise2"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("PaleTurquoise3"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("PaleTurquoise4"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("powder blue"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("cornflower blue"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("deep sky blue"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("DeepSkyBlue2"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("DeepSkyBlue3"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("DeepSkyBlue4"));

  row = 0;
  column++;
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("light blue"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("LightBlue1"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("LightBlue2"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("LightBlue3"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("LightBlue4"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("light steel blue"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("LightSteelBlue1"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("LightSteelBlue2"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("LightSteelBlue3"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("LightSteelBlue4"));
  m_x11ColorTbl1->setItem(
    row++, column, getX11Color1Item("dark slate gray", "white"));
  m_x11ColorTbl1->setItem(
    row++, column, getX11Color1Item("DarkSlateGray1", "white"));
  m_x11ColorTbl1->setItem(
    row++, column, getX11Color1Item("DarkSlateGray2", "white"));
  m_x11ColorTbl1->setItem(
    row++, column, getX11Color1Item("DarkSlateGray3", "white"));
  m_x11ColorTbl1->setItem(
    row++, column, getX11Color1Item("DarkSlateGray4", "white"));

  row = 0;
  column++;
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("light sky blue"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("LightSkyBlue1"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("LightSkyBlue2"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("LightSkyBlue3"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("LightSkyBlue4"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("sky blue"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("SkyBlue2"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("SkyBlue3"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("SkyBlue4"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("SteelBlue1"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("SteelBlue2"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("SteelBlue3"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("steel blue"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("SteelBlue4"));

  row = 0;
  column++;
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("royal blue"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("RoyalBlue1"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("RoyalBlue2"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("RoyalBlue3"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("RoyalBlue4"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("blue", "white"));
  m_x11ColorTbl1->setItem(
    row++, column, getX11Color1Item("medium blue", "white"));
  m_x11ColorTbl1->setItem(
    row++, column, getX11Color1Item("dark blue", "white"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("navy", "white"));
  m_x11ColorTbl1->setItem(
    row++, column, getX11Color1Item("midnight blue", "white"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("medium slate blue"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("slate blue"));
  m_x11ColorTbl1->setItem(
    row++, column, getX11Color1Item("dark slate blue", "white"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("dodger blue"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("DodgerBlue2"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("DodgerBlue3"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("DodgerBlue4"));

  row = 0;
  column++;
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("lime green"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("dark sea green"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("pale green"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("light green"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("spring green"));
  m_x11ColorTbl1->setItem(
    row++, column, getX11Color1Item("forest green", "white"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("green", "white"));
  m_x11ColorTbl1->setItem(
    row++, column, getX11Color1Item("dark green", "white"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("medium sea green"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("sea green"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("aquamarine"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("aquamarine2"));

  row = 0;
  column++;
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("aquamarine3"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("aquamarine4"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("medium aquamarine"));
  m_x11ColorTbl1->setItem(
    row++, column, getX11Color1Item("medium spring green"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("green yellow"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("lawn green"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("chartreuse"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("chartreuse2"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("chartreuse3"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("chartreuse4"));
  m_x11ColorTbl1->setItem(row++, column, getX11Color1Item("yellow green"));
  m_x11ColorTbl1->setItem(
    row++, column, getX11Color1Item("olive drab", "white"));
  m_x11ColorTbl1->setItem(
    row++, column, getX11Color1Item("dark olive green", "white"));

  return frame;
}

QFrame*
ExtendedColorDialog::initX11ColorFrame2()
{
  auto frame = new QFrame(this);
  auto layout = new QGridLayout;
  frame->setLayout(layout);

  m_x11ColorTbl2 = new QTableWidget(22, 10, this);
  m_x11ColorTbl2->setStyleSheet("QTableView { gridline-color: white; }");
  m_x11ColorTbl2->horizontalHeader()->setVisible(false);
  m_x11ColorTbl2->horizontalHeader()->setSectionResizeMode(
    QHeaderView::ResizeToContents);
  m_x11ColorTbl2->verticalHeader()->setVisible(false);
  layout->addWidget(m_x11ColorTbl2);
  connect(m_x11ColorTbl2,
          &QTableWidget::cellClicked,
          this,
          &ExtendedColorDialog::selectX11_2Color);

  layout->addWidget(m_col2SelectionLbl = createColorDisplay(), 1, 0);

  layout->addWidget(createBtnBox(), 2, 0);

  int row = 0, column = 0;
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("cyan"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("cyan2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("cyan3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("cyan4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("turquoise"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("turquoise1"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("turquoise2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("turquoise3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("turquoise4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("dark turquoise"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("light sea green"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("dark cyan"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("seashell1"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("seashell2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("seashell3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("seashell4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("lavender"));

  row = 0;
  column++;
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LavenderBlush1"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LavenderBlush2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LavenderBlush3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LavenderBlush4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("misty rose"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("MistyRose2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("MistyRose3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("MistyRose4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LightSalmon1"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LightSalmon2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LightSalmon3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LightSalmon4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("dark salmon"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("salmon1"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("salmon2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("salmon3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("salmon4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("tomato1"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("tomato2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("tomato3"));

  row = 0;
  column++;
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("tomato4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("red1", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("red2", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("red3", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("red4", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("pink1"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("pink2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("pink3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("pink4"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("indian red", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("IndianRed1", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("IndianRed2", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("IndianRed3", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("IndianRed4", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("hot pink"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("HotPink1"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("HotPink2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("HotPink3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("HotPink4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("deep pink"));

  row = 0;
  column++;
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("DeepPink2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("DeepPink3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("DeepPink4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("light pink"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LightPink1"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LightPink2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LightPink3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LightPink4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("pale violet red"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("PaleVioletRed1"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("PaleVioletRed2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("PaleVioletRed3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("PaleVioletRed4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("magenta"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("magenta2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("magenta3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("magenta4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("medium violet red"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("thistle"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("thistle2"));

  row = 0;
  column++;
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("thistle3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("thistle4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("plum"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("plum2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("plum3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("plum4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("violet"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("orchid"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("orchid2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("orchid3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("orchid4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("medium orchid"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("MediumOrchid1"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("MediumOrchid2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("MediumOrchid3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("MediumOrchid4"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("dark magenta", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("purple", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("purple2", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("purple3", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("purple4", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("medium purple"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("MediumPurple1"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("MediumPurple2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("MediumPurple3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("MediumPurple4"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("dark orchid", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("DarkOrchid1", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("DarkOrchid2", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("DarkOrchid3", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("DarkOrchid4", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("dark violet", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("ivory"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("ivory2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("ivory3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("ivory4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("beige"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("lightyellow"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LightYellow2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LightYellow3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LightYellow4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("pale goldenrod"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("cornsilk"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("cornsilk2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("cornsilk3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("cornsilk4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("lemonchiffon"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LemonChiffon2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LemonChiffon3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("LemonChiffon4"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("light goldenrod yellow"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("moccasin"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("wheat"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("khaki"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("khaki2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("khaki3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("khaki4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("dark khaki"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("gold"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("gold2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("gold3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("gold4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("goldenrod"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("goldenrod"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("goldenrod2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("goldenrod3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("goldenrod4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("dark goldenrod"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("yellow"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("yellow2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("yellow3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("yellow4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("orange"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("orange2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("orange3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("orange4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("dark orange"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("DarkOrange1"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("DarkOrange2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("DarkOrange3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("DarkOrange4"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("orange red", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("OrangeRed2", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("OrangeRed3", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("OrangeRed4", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("floral white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("oldlace"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("antique white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("AntiqueWhite1"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("AntiqueWhite2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("AntiqueWhite3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("AntiqueWhite4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("peachpuff"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("peachpuff"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("peachpuff"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("peachpuff"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("peachpuff"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("bisque"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("bisque2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("bisque3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("bisque4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("navajowhite"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("NavajoWhite2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("NavajoWhite3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("NavajoWhite4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("burlywood"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("burlywood2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("burlywood3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("burlywood4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("tan2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("tan3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("tan4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("sandy brown"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("sienna", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("sienna2", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("sienna3", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("sienna4", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("chocolate", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("chocolate2", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("chocolate3", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("chocolate4", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("saddle brown", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("coral"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("coral2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("coral3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("coral4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("light coral"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("rosy brown"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("RosyBrown1"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("RosyBrown2"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("RosyBrown3"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("RosyBrown4"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("brown", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("brown2", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("brown3", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("brown4", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("firebrick", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("firebrick2", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("firebrick3", "white"));
  m_x11ColorTbl2->setItem(
    row++, column, getX11Color2Item("firebrick4", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("dark red", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("maroon", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("maroon2", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("maroon3", "white"));
  m_x11ColorTbl2->setItem(row++, column, getX11Color2Item("maroon4", "white"));

  return frame;
}

QFrame*
ExtendedColorDialog::initX11MonoFrame()
{
  auto frame = new QFrame(this);
  auto layout = new QGridLayout;
  frame->setLayout(layout);

  m_x11MonoTbl = new QTableWidget(17, 10, this);
  m_x11MonoTbl->setStyleSheet("QTableView { gridline-color: white; }");
  m_x11MonoTbl->horizontalHeader()->setVisible(false);
  m_x11MonoTbl->verticalHeader()->setVisible(false);
  m_x11MonoTbl->horizontalHeader()->setSectionResizeMode(
    QHeaderView::ResizeToContents);
  m_x11MonoTbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(m_x11MonoTbl);
  connect(m_x11MonoTbl,
          &QTableWidget::cellClicked,
          this,
          &ExtendedColorDialog::selectX11_Mono);

  layout->addWidget(m_monoSelectionLbl = createColorDisplay(), 1, 0);
  layout->addWidget(createBtnBox(), 2, 0);

  auto row = 0;
  auto column = 0;
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey0", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey1", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey2", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey3", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey4", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey5", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey6", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey7", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey8", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey9", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey10", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey11", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey12", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey13", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey14", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey15", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey16", "white"));

  row = 0;
  column++;
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey17", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey18", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey19", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey20", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey21", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey22", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey23", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey24", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey25", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey26", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey27", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey28", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey29", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey30", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey31", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey32", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey33", "white"));

  row = 0;
  column++;
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey34", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey35", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey36", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey37", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey38", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey39", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey40"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey41"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey42"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey43"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey44"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey45"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey46"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey47"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey48"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey49"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey50"));

  row = 0;
  column++;
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey51"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey52"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey53"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey54"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey55"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey56"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey57"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey58"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey59"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey60"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey61"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey62"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey63"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey64"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey65"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey66"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey67"));

  row = 0;
  column++;
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey68"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey69"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey70"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey71"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey72"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey73"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey74"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey75"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey76"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey77"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey78"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey79"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey80"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey81"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey82"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey83"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey84"));

  row = 0;
  column++;
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey85"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey86"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey87"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey88"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey89"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey90"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey91"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey92"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey93"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey94"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey95"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey96"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey97"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey98"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey99"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("grey100"));

  row = 0;
  column++;
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("snow"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("ghost white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("white smoke"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("snow2"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("snow3"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("snow4"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("gainsboro"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("light gray"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("dark gray"));
  m_x11MonoTbl->setItem(
    row++, column, getX11MonoItem("light slate gray", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("slate gray", "white"));

  row = 0;
  column++;
  m_x11MonoTbl->setItem(row++, column, getX11Color1Item("honeydew"));
  m_x11MonoTbl->setItem(row++, column, getX11Color1Item("honeydew2"));
  m_x11MonoTbl->setItem(row++, column, getX11Color1Item("honeydew3"));
  m_x11MonoTbl->setItem(row++, column, getX11Color1Item("honeydew4"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("gray", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("dim gray", "white"));
  m_x11MonoTbl->setItem(row++, column, getX11MonoItem("black", "white"));

  row = 0;
  column++;

  return frame;
}

QColor
ExtendedColorDialog::color() const
{
  return m_color;
}

QString
ExtendedColorDialog::rgb(int alpha) const
{
  QString name;
  if (alpha >= 0 && alpha <= 100) {
    name = ExtendedColorDialog::RGBACOLOR.arg(m_color.red())
             .arg(m_color.green())
             .arg(m_color.blue())
             .arg(alpha);
  } else {
    name = ExtendedColorDialog::RGBCOLOR.arg(m_color.red())
             .arg(m_color.green())
             .arg(m_color.blue());
  }
  if (name.isEmpty() && m_color.isValid())
    return hash(alpha);
  return name;
}

QString
ExtendedColorDialog::hsv(int alpha) const
{
  QString name;
  if (alpha >= 0 && alpha <= 100) {
    name = ExtendedColorDialog::HSVACOLOR.arg(m_color.hsvHue())
             .arg(m_color.hsvSaturation())
             .arg(m_color.value())
             .arg(alpha);
  } else {
    name = ExtendedColorDialog::HSVCOLOR.arg(m_color.hsvHue())
             .arg(m_color.hsvSaturation())
             .arg(m_color.value());
  }
  if (name.isEmpty() && m_color.isValid())
    return hash(alpha);
  return name;
}

QString
ExtendedColorDialog::hsl(int alpha) const
{
  QString name;
  if (alpha >= 0 && alpha <= 100) {
    name = ExtendedColorDialog::HSVACOLOR.arg(m_color.hslHue())
             .arg(m_color.hslSaturation())
             .arg(m_color.value())
             .arg(alpha);
  } else {
    name = ExtendedColorDialog::HSVCOLOR.arg(m_color.hslHue())
             .arg(m_color.hslSaturation())
             .arg(m_color.value());
  }
  if (name.isEmpty() && m_color.isValid())
    return hash(alpha);
  return name;
}

QString
ExtendedColorDialog::name() const
{
  if (m_name.isEmpty()) {
    QString name = QColorConstants::X11::name(m_color);
    if (name.isEmpty()) {
      return hash();
    }
  }
  return m_name;
}

QString
ExtendedColorDialog::hash(int alpha) const
{
  if (m_color.isValid()) {
    if (alpha >= 0 && alpha <= 100)
      return m_color.name(QColor::HexArgb);
    return m_color.name(QColor::HexRgb);
  }
  return QString();
}

QSize
ExtendedColorDialog::sizeHint() const
{
  return QSize(900, 1500);
}

void
ExtendedColorDialog::acceptColor()
{
  m_color = m_colorDlg->currentColor();
  accept();
}

void
ExtendedColorDialog::acceptStandardColor()
{
  if (m_color.isValid()) {
    emit currentColorChanged(m_color);
    accept();
  } else {
    reject();
  }
}

ColorDropLabel::ColorDropLabel(QWidget* parent)
  : QWidget(parent)
{
  auto layout = new QHBoxLayout;
  layout->setSpacing(0);
  setLayout(layout);
  m_left = new QLabel(this);
  layout->addWidget(m_left);
  m_right = new QLabel(this);
  layout->addWidget(m_right);
}

void
ColorDropLabel::setPrimaryColor(const QColor& color)
{
  m_color = color;
  if (m_dropColor.isValid()) {
    m_left->setStyleSheet(colorToString(color));
  } else {
    auto sheet = colorToString(color);
    m_right->setStyleSheet(sheet);
    m_left->setStyleSheet(sheet);
  }
}

void
ColorDropLabel::setSecondaryColor(const QColor& color)
{
  m_dropColor = color;
  m_left->setStyleSheet(colorToString(color));
}

void
ColorDropLabel::dragEnterEvent(QDragEnterEvent* event)
{
  if (event->mimeData()->hasColor())
    event->accept();
  else
    event->ignore();
}

void
ColorDropLabel::dropEvent(QDropEvent* event)
{
  if (event->mimeData()->hasColor()) {
    QColor color = event->mimeData()->colorData().value<QColor>();
    auto sheet = colorToString(color);
    m_right->setStyleSheet(sheet);
  }
}

QString
ColorDropLabel::colorToString(QColor color)
{
  if (color.isValid()) {
    auto red = QStringLiteral("%1").arg(color.red(), 2, 16, QLatin1Char('0'));
    auto green =
      QStringLiteral("%1").arg(color.green(), 2, 16, QLatin1Char('0'));
    auto blue = QStringLiteral("%1").arg(color.blue(), 2, 16, QLatin1Char('0'));
    auto value = QString("#%1%2%3").arg(red).arg(green).arg(blue);
    return DISPLAYLABELSTYLE.arg(value);
  }
  return DISPLAYLABELSTYLE.arg("white");
}

ColorDragWidget::ColorDragWidget(QWidget* parent)
  : QTableWidget(parent)
{}

void
ColorDragWidget::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    auto pos = event->pos();
    auto item = itemAt(pos);
    if (item) {
      m_dragStartPosition = pos;
      m_color = item->background().color();
    } else {
      m_color = QColor();
    }
  }
  QTableWidget::mousePressEvent(event);
}

void
ColorDragWidget::mouseMoveEvent(QMouseEvent* event)
{
  if (!(event->buttons() & Qt::LeftButton))
    return;
  if ((event->pos() - m_dragStartPosition).manhattanLength() <
      QApplication::startDragDistance())
    return;

  if (m_color.isValid()) {
    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData;
    mimeData->setColorData(m_color);
    drag->setMimeData(mimeData);
    /*Qt::DropAction dropAction =*/drag->exec(Qt::CopyAction | Qt::MoveAction);
  }
  QTableWidget::mouseMoveEvent(event);
}
