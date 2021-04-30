#include "stylesheetedit/extendedcolordialog.h"
#include "stylesheetedit/x11colors.h"

#include <QtDebug>

const QString ColorDropDisplay::DISPLAYLABELRIGHT =
  "QLabel {"
  "background-color: %1;"
  "border: 0px;"
  "border-top-left-radius: 0px;"
  "border-bottom-left-radius: 0px;"
  "border-top-right-radius: 25px;"
  "border-bottom-right-radius: 25px"
  "}";
const QString ColorDropDisplay::DISPLAYLABELLEFT =
  "QLabel {"
  "background-color: %1;"
  "border: 0px;"
  "border-top-left-radius: 25px;"
  "border-bottom-left-radius: 25px;"
  "border-top-right-radius: 0px;"
  "border-bottom-right-radius: 0px"
  "}";
const QString ColorDropDisplay::DISPLAYBORDER =
  "QFrame {"
  "border: 2px solid darkslategray; "
  "border-radius: 25px;"
  "}";
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
  , m_color(QColor(Qt::white))
  , m_dropColor(QColor(Qt::white))
{
  initGui();
  setColor(QColor(Qt::white));
  //  setSecondaryColor(QColor(Qt::white));
}

ExtendedColorDialog::ExtendedColorDialog(const QColor& initialColor,
                                         QWidget* parent)
  : QDialog(parent)
  , m_color(initialColor)
  , m_dropColor(QColor(Qt::white))
{
  initGui();
  setColor(initialColor);
  //  setSecondaryColor(initialColor);
}

ExtendedColorDialog::ExtendedColorDialog(const QColor& initialColor,
                                         const QColor& secondaryColor,
                                         QWidget* parent)
  : QDialog(parent)
  , m_color(initialColor)
  , m_dropColor(secondaryColor)
{
  initGui();
  setColor(m_color);
  setSecondaryColor(m_dropColor);
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

  m_currentTab = SvgTab;
  m_tabs->setCurrentIndex(int(m_currentTab));

  connect(m_tabs,
          &QTabWidget::currentChanged,
          this,
          &ExtendedColorDialog::tabChanged);
}

// void
// ExtendedColorDialog::table->setData(int row,
//                                int column,
//                                const QString& back,
//                                const QString& fore)
//{
//  auto background = QColorConstants::Svg::color(back);
//  if (!fore.isEmpty()) {
//    auto foreground = QColorConstants::Svg::color(fore);
//    m_svgTable->setData(row, column, back, background, foreground);
//  } else {
//    m_svgTable->setData(row, column, back, background);
//  }
//}

// void
// ExtendedColorDialog::table->setData(ColorDragTable* table,
//                                int row,
//                                int column,
//                                const QString& back,
//                                const QString& fore)
//{
//  auto background = QColorConstants::X11::color(back);
//  if (!fore.isEmpty()) {
//    auto foreground = QColorConstants::X11::color(fore);
//    table->setData(row, column, back, background, foreground);
//  } else {
//    table->setData(row, column, back, background);
//  }
//}

void
ExtendedColorDialog::colorClicked(const QModelIndex& index)
{
  auto table = qobject_cast<ColorDragTable*>(sender());
  if (index.isValid()) {
    m_color = table->background(index);
    m_name = table->name(index);
    table->label()->setCurrentColor(m_color);
    emit colorSelected(m_color);
  }
}

// void
// ExtendedColorDialog::x11Color1Clicked(const QModelIndex& index)
//{
//  if (index.isValid()) {
//    m_color = m_x11Color1Tbl->background(index);
//    m_name = m_x11Color1Tbl->name(index);
//    m_x11Color1Lbl->setCurrentColor(m_color);
//    emit colorSelected(m_color);
//  }
//}

// void
// ExtendedColorDialog::selectX11_2Color(const QModelIndex& index)
//{
//  if (index.isValid()) {
//    m_color = m_x11Color2Tbl->background(index);
//    m_name = m_x11Color2Tbl->name(index);
//    m_x11Color2Lbl->setCurrentColor(m_color);
//    emit colorSelected(m_color);
//  }
//}

// void
// ExtendedColorDialog::selectX11_Mono(const QModelIndex& index)
//{
//  if (index.isValid()) {
//    m_color = m_x11MonoTbl->background(index);
//    m_name = m_x11MonoTbl->name(index);
//    m_x11MonoLbl->setCurrentColor(m_color);
//    emit colorSelected(m_color);
//  }
//}

void
ExtendedColorDialog::tabChanged(int index)
{
  auto widget = m_tabs->widget(int(m_currentTab));
  auto frame = qobject_cast<ColorDragFrame*>(widget);
  if (frame) {
    m_color = frame->getDisplay()->color();
    m_dropColor = frame->getDisplay()->dropColor();
  } else {
    m_color = m_colorDlg->currentColor();
  }

  auto newWidget = m_tabs->widget(index);
  auto newFrame = qobject_cast<ColorDragFrame*>(newWidget);
  if (newFrame) {
    frame->getDisplay()->setCurrentColor(m_color);
    frame->getDisplay()->setSecondaryColor(m_dropColor);
  } else {
    m_colorDlg->setCurrentColor(m_color);
  }

  m_currentTab = Tabs(index);
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

ColorDropDisplay*
ExtendedColorDialog::createColorDisplay()
{
  auto lbl = new ColorDropDisplay(m_color, m_dropColor, this);
  lbl->setMinimumSize(QSize(300, 90));
  lbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  return lbl;
}

ColorDragTable*
ExtendedColorDialog::createColorTable(ColorDragFrame* frame)
{
  auto table = new ColorDragTable(20, 7, this);
  auto display = createColorDisplay();
  table->setLabel(display);
  table->setSelectionMode(QTableView::SingleSelection);
  table->setSelectionBehavior(QTableView::SelectItems);
  table->setDragDropMode(QTableView::DragOnly);
  table->horizontalHeader()->setVisible(false);
  table->verticalHeader()->setVisible(false);
  table->horizontalHeader()->setSectionResizeMode(
    QHeaderView::ResizeToContents);
  table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  connect(
    table, &ColorDragTable::clicked, this, &ExtendedColorDialog::colorClicked);
  frame->layout()->addWidget(table, 0, 0);
  frame->layout()->addWidget(display, 1, 0);
  return table;
}

QFrame*
ExtendedColorDialog::initSvgFrame()
{
  auto frame = new ColorDragFrame(this);
  auto layout = new QGridLayout;
  frame->setLayout(layout);

  auto table = createColorTable(frame);
  layout->addWidget(createBtnBox(), 2, 0);

  int row = 0, column = 0;
  // blues
  table->setData(row++, column, "aliceblue");
  table->setData(row++, column, "azure");
  table->setData(row++, column, "lightcyan");
  table->setData(row++, column, "paleturquoise");
  table->setData(row++, column, "powderblue");
  table->setData(row++, column, "lightblue");
  table->setData(row++, column, "lightblue");
  table->setData(row++, column, "lightskyblue");
  table->setData(row++, column, "skyblue");
  table->setData(row++, column, "cornflowerblue");
  table->setData(row++, column, "steelblue");
  table->setData(row++, column, "dodgerblue");
  table->setData(row++, column, "royalblue");
  table->setData(row++, column, "deepskyblue");
  table->setData(row++, column, "mediumslateblue");
  table->setData(row++, column, "slateblue");
  table->setData(row++, column, "indigo", "white");
  table->setData(row++, column, "darkslateblue", "white");
  table->setData(row++, column, "mediumblue", "white");
  table->setData(row++, column, "darkblue", "white");

  row = 0;
  column++;
  table->setData(row++, column, "navy", "white");
  table->setData(row++, column, "blue", "white");
  table->setData(row++, column, "midnightblue", "white");
  table->setData(row++, column, "honeydew");
  table->setData(row++, column, "limegreen");
  table->setData(row++, column, "darkseagreen");
  table->setData(row++, column, "palegreen");
  table->setData(row++, column, "lightgreen");
  table->setData(row++, column, "springgreen");
  table->setData(row++, column, "lime");
  table->setData(row++, column, "forestgreen", "white");
  table->setData(row++, column, "green", "white");
  table->setData(row++, column, "darkgreen", "white");
  table->setData(row++, column, "aquamarine");
  table->setData(row++, column, "mediumaquamarine");
  table->setData(row++, column, "mediumseagreen");
  table->setData(row++, column, "mediumspringgreen");
  table->setData(row++, column, "seagreen");
  table->setData(row++, column, "greenyellow");
  table->setData(row++, column, "lawngreen");

  row = 0;
  column++;
  table->setData(row++, column, "chartreuse");
  table->setData(row++, column, "yellowgreen");
  table->setData(row++, column, "olivedrab", "white");
  table->setData(row++, column, "olive", "white");
  table->setData(row++, column, "darkolivegreen", "white");
  table->setData(row++, column, "cyan");
  table->setData(row++, column, "aqua");
  table->setData(row++, column, "turquoise");
  table->setData(row++, column, "mediumturquoise");
  table->setData(row++, column, "darkturquoise");
  table->setData(row++, column, "lightseagreen");
  table->setData(row++, column, "cadetblue");
  table->setData(row++, column, "darkcyan");
  table->setData(row++, column, "teal");
  table->setData(row++, column, "seashell");
  table->setData(row++, column, "lavenderblush");
  table->setData(row++, column, "mistyrose");
  table->setData(row++, column, "lightsalmon");
  table->setData(row++, column, "darksalmon");
  table->setData(row++, column, "salmon");

  row = 0;
  column++;
  table->setData(row++, column, "tomato");
  table->setData(row++, column, "red", "white");
  table->setData(row++, column, "pink");
  table->setData(row++, column, "crimson", "white");
  table->setData(row++, column, "indianred", "white");
  table->setData(row++, column, "lavender");
  table->setData(row++, column, "hotpink");
  table->setData(row++, column, "lightpink");
  table->setData(row++, column, "palevioletred");
  table->setData(row++, column, "fuchsia");
  table->setData(row++, column, "magenta");
  table->setData(row++, column, "deeppink");
  table->setData(row++, column, "mediumvioletred");
  table->setData(row++, column, "thistle");
  table->setData(row++, column, "plum");
  table->setData(row++, column, "violet");
  table->setData(row++, column, "orchid");
  table->setData(row++, column, "mediumorchid");
  table->setData(row++, column, "darkmagenta", "white");
  table->setData(row++, column, "purple", "white");

  row = 0;
  column++;
  table->setData(row++, column, "mediumpurple");
  table->setData(row++, column, "darkorchid", "white");
  table->setData(row++, column, "darkviolet", "white");
  table->setData(row++, column, "blueviolet", "white");
  table->setData(row++, column, "ivory");
  table->setData(row++, column, "beige");
  table->setData(row++, column, "lightyellow");
  table->setData(row++, column, "linen");
  table->setData(row++, column, "palegoldenrod");
  table->setData(row++, column, "cornsilk");
  table->setData(row++, column, "floralwhite");
  table->setData(row++, column, "lemonchiffon");
  table->setData(row++, column, "lightgoldenrodyellow");
  table->setData(row++, column, "moccasin");
  table->setData(row++, column, "wheat");
  table->setData(row++, column, "khaki");
  table->setData(row++, column, "darkkhaki");
  table->setData(row++, column, "gold");
  table->setData(row++, column, "goldenrod");
  table->setData(row++, column, "darkgoldenrod");

  row = 0;
  column++;
  table->setData(row++, column, "yellow");
  table->setData(row++, column, "orange");
  table->setData(row++, column, "darkorange");
  table->setData(row++, column, "orangered", "white");
  table->setData(row++, column, "oldlace");
  table->setData(row++, column, "antiquewhite");
  table->setData(row++, column, "papayawhip");
  table->setData(row++, column, "peachpuff");
  table->setData(row++, column, "bisque");
  table->setData(row++, column, "navajowhite");
  table->setData(row++, column, "blanchedalmond");
  table->setData(row++, column, "burlywood");
  table->setData(row++, column, "tan");
  table->setData(row++, column, "sandybrown");
  table->setData(row++, column, "peru", "white");
  table->setData(row++, column, "sienna", "white");
  table->setData(row++, column, "chocolate", "white");
  table->setData(row++, column, "saddlebrown", "white");
  table->setData(row++, column, "coral");
  table->setData(row++, column, "lightcoral");

  row = 0;
  column++;
  table->setData(row++, column, "rosybrown");
  table->setData(row++, column, "brown", "white");
  table->setData(row++, column, "firebrick", "white");
  table->setData(row++, column, "darkred", "white");
  table->setData(row++, column, "maroon", "white");
  table->setData(row++, column, "snow");
  table->setData(row++, column, "white");
  table->setData(row++, column, "whitesmoke");
  table->setData(row++, column, "mintcream");
  table->setData(row++, column, "ghostwhite");
  table->setData(row++, column, "gainsboro");
  table->setData(row++, column, "lightgray");
  table->setData(row++, column, "silver");
  table->setData(row++, column, "darkgray");
  table->setData(row++, column, "lightslategray", "white");
  table->setData(row++, column, "slategray", "white");
  table->setData(row++, column, "darkslategray", "white");
  table->setData(row++, column, "gray", "white");
  table->setData(row++, column, "dimgray", "white");
  table->setData(row++, column, "black", "white");

  return frame;
}

QFrame*
ExtendedColorDialog::initX11ColorFrame1()
{
  auto frame = new ColorDragFrame(this);
  auto layout = new QGridLayout;
  frame->setLayout(layout);

  auto table = createColorTable(frame);
  layout->addWidget(createBtnBox(), 2, 0);

  int row = 0, column = 0;
  table->setData(row++, column, "mint cream");
  table->setData(row++, column, "mint cream");
  table->setData(row++, column, "alice blue");
  table->setData(row++, column, "azure");
  table->setData(row++, column, "azure2");
  table->setData(row++, column, "azure3");
  table->setData(row++, column, "azure4");
  table->setData(row++, column, "light cyan");
  table->setData(row++, column, "pale turquoise");
  table->setData(row++, column, "PaleTurquoise1");
  table->setData(row++, column, "PaleTurquoise2");
  table->setData(row++, column, "PaleTurquoise3");
  table->setData(row++, column, "PaleTurquoise4");
  table->setData(row++, column, "powder blue");
  table->setData(row++, column, "cornflower blue");
  table->setData(row++, column, "deep sky blue");
  table->setData(row++, column, "DeepSkyBlue2");
  table->setData(row++, column, "DeepSkyBlue3");
  table->setData(row++, column, "DeepSkyBlue4");

  row = 0;
  column++;
  table->setData(row++, column, "light blue");
  table->setData(row++, column, "LightBlue1");
  table->setData(row++, column, "LightBlue2");
  table->setData(row++, column, "LightBlue3");
  table->setData(row++, column, "LightBlue4");
  table->setData(row++, column, "light steel blue");
  table->setData(row++, column, "LightSteelBlue1");
  table->setData(row++, column, "LightSteelBlue2");
  table->setData(row++, column, "LightSteelBlue3");
  table->setData(row++, column, "LightSteelBlue4");
  table->setData(row++, column, "dark slate gray", "white");
  table->setData(row++, column, "DarkSlateGray1", "white");
  table->setData(row++, column, "DarkSlateGray2", "white");
  table->setData(row++, column, "DarkSlateGray3", "white");
  table->setData(row++, column, "DarkSlateGray4", "white");

  row = 0;
  column++;
  table->setData(row++, column, "light sky blue");
  table->setData(row++, column, "LightSkyBlue1");
  table->setData(row++, column, "LightSkyBlue2");
  table->setData(row++, column, "LightSkyBlue3");
  table->setData(row++, column, "LightSkyBlue4");
  table->setData(row++, column, "sky blue");
  table->setData(row++, column, "SkyBlue2");
  table->setData(row++, column, "SkyBlue3");
  table->setData(row++, column, "SkyBlue4");
  table->setData(row++, column, "SteelBlue1");
  table->setData(row++, column, "SteelBlue2");
  table->setData(row++, column, "SteelBlue3");
  table->setData(row++, column, "steel blue");
  table->setData(row++, column, "SteelBlue4");

  row = 0;
  column++;
  table->setData(row++, column, "royal blue");
  table->setData(row++, column, "RoyalBlue1");
  table->setData(row++, column, "RoyalBlue2");
  table->setData(row++, column, "RoyalBlue3");
  table->setData(row++, column, "RoyalBlue4");
  table->setData(row++, column, "blue", "white");
  table->setData(row++, column, "medium blue", "white");
  table->setData(row++, column, "dark blue", "white");
  table->setData(row++, column, "navy", "white");
  table->setData(row++, column, "midnight blue", "white");
  table->setData(row++, column, "medium slate blue");
  table->setData(row++, column, "slate blue");
  table->setData(row++, column, "dark slate blue", "white");
  table->setData(row++, column, "dodger blue");
  table->setData(row++, column, "DodgerBlue2");
  table->setData(row++, column, "DodgerBlue3");
  table->setData(row++, column, "DodgerBlue4");

  row = 0;
  column++;
  table->setData(row++, column, "lime green");
  table->setData(row++, column, "dark sea green");
  table->setData(row++, column, "pale green");
  table->setData(row++, column, "light green");
  table->setData(row++, column, "spring green");
  table->setData(row++, column, "forest green", "white");
  table->setData(row++, column, "green", "white");
  table->setData(row++, column, "dark green", "white");
  table->setData(row++, column, "medium sea green");
  table->setData(row++, column, "sea green");
  table->setData(row++, column, "aquamarine");
  table->setData(row++, column, "aquamarine2");

  row = 0;
  column++;
  table->setData(row++, column, "aquamarine3");
  table->setData(row++, column, "aquamarine4");
  table->setData(row++, column, "medium aquamarine");
  table->setData(row++, column, "medium spring green");
  table->setData(row++, column, "green yellow");
  table->setData(row++, column, "lawn green");
  table->setData(row++, column, "chartreuse");
  table->setData(row++, column, "chartreuse2");
  table->setData(row++, column, "chartreuse3");
  table->setData(row++, column, "chartreuse4");
  table->setData(row++, column, "yellow green");
  table->setData(row++, column, "olive drab", "white");
  table->setData(row++, column, "dark olive green", "white");

  return frame;
}

QFrame*
ExtendedColorDialog::initX11ColorFrame2()
{
  auto frame = new ColorDragFrame(this);
  auto layout = new QGridLayout;
  frame->setLayout(layout);

  auto table = createColorTable(frame);
  layout->addWidget(createBtnBox(), 2, 0);

  int row = 0, column = 0;
  table->setData(row++, column, "cyan");
  table->setData(row++, column, "cyan2");
  table->setData(row++, column, "cyan3");
  table->setData(row++, column, "cyan4");
  table->setData(row++, column, "turquoise");
  table->setData(row++, column, "turquoise1");
  table->setData(row++, column, "turquoise2");
  table->setData(row++, column, "turquoise3");
  table->setData(row++, column, "turquoise4");
  table->setData(row++, column, "dark turquoise");
  table->setData(row++, column, "light sea green");
  table->setData(row++, column, "dark cyan");
  table->setData(row++, column, "seashell1");
  table->setData(row++, column, "seashell2");
  table->setData(row++, column, "seashell3");
  table->setData(row++, column, "seashell4");
  table->setData(row++, column, "lavender");

  row = 0;
  column++;
  table->setData(row++, column, "LavenderBlush1");
  table->setData(row++, column, "LavenderBlush2");
  table->setData(row++, column, "LavenderBlush3");
  table->setData(row++, column, "LavenderBlush4");
  table->setData(row++, column, "misty rose");
  table->setData(row++, column, "MistyRose2");
  table->setData(row++, column, "MistyRose3");
  table->setData(row++, column, "MistyRose4");
  table->setData(row++, column, "LightSalmon1");
  table->setData(row++, column, "LightSalmon2");
  table->setData(row++, column, "LightSalmon3");
  table->setData(row++, column, "LightSalmon4");
  table->setData(row++, column, "dark salmon");
  table->setData(row++, column, "salmon1");
  table->setData(row++, column, "salmon2");
  table->setData(row++, column, "salmon3");
  table->setData(row++, column, "salmon4");
  table->setData(row++, column, "tomato1");
  table->setData(row++, column, "tomato2");
  table->setData(row++, column, "tomato3");

  row = 0;
  column++;
  table->setData(row++, column, "tomato4");
  table->setData(row++, column, "red1", "white");
  table->setData(row++, column, "red2", "white");
  table->setData(row++, column, "red3", "white");
  table->setData(row++, column, "red4", "white");
  table->setData(row++, column, "pink1");
  table->setData(row++, column, "pink2");
  table->setData(row++, column, "pink3");
  table->setData(row++, column, "pink4");
  table->setData(row++, column, "indian red", "white");
  table->setData(row++, column, "IndianRed1", "white");
  table->setData(row++, column, "IndianRed2", "white");
  table->setData(row++, column, "IndianRed3", "white");
  table->setData(row++, column, "IndianRed4", "white");
  table->setData(row++, column, "hot pink");
  table->setData(row++, column, "HotPink1");
  table->setData(row++, column, "HotPink2");
  table->setData(row++, column, "HotPink3");
  table->setData(row++, column, "HotPink4");
  table->setData(row++, column, "deep pink");

  row = 0;
  column++;
  table->setData(row++, column, "DeepPink2");
  table->setData(row++, column, "DeepPink3");
  table->setData(row++, column, "DeepPink4");
  table->setData(row++, column, "light pink");
  table->setData(row++, column, "LightPink1");
  table->setData(row++, column, "LightPink2");
  table->setData(row++, column, "LightPink3");
  table->setData(row++, column, "LightPink4");
  table->setData(row++, column, "pale violet red");
  table->setData(row++, column, "PaleVioletRed1");
  table->setData(row++, column, "PaleVioletRed2");
  table->setData(row++, column, "PaleVioletRed3");
  table->setData(row++, column, "PaleVioletRed4");
  table->setData(row++, column, "magenta");
  table->setData(row++, column, "magenta2");
  table->setData(row++, column, "magenta3");
  table->setData(row++, column, "magenta4");
  table->setData(row++, column, "medium violet red");
  table->setData(row++, column, "thistle");
  table->setData(row++, column, "thistle2");

  row = 0;
  column++;
  table->setData(row++, column, "thistle3");
  table->setData(row++, column, "thistle4");
  table->setData(row++, column, "plum");
  table->setData(row++, column, "plum2");
  table->setData(row++, column, "plum3");
  table->setData(row++, column, "plum4");
  table->setData(row++, column, "violet");
  table->setData(row++, column, "orchid");
  table->setData(row++, column, "orchid2");
  table->setData(row++, column, "orchid3");
  table->setData(row++, column, "orchid4");
  table->setData(row++, column, "medium orchid");
  table->setData(row++, column, "MediumOrchid1");
  table->setData(row++, column, "MediumOrchid2");
  table->setData(row++, column, "MediumOrchid3");
  table->setData(row++, column, "MediumOrchid4");
  table->setData(row++, column, "dark magenta", "white");
  table->setData(row++, column, "purple", "white");
  table->setData(row++, column, "purple2", "white");
  table->setData(row++, column, "purple3", "white");
  table->setData(row++, column, "purple4", "white");
  table->setData(row++, column, "medium purple");
  table->setData(row++, column, "MediumPurple1");
  table->setData(row++, column, "MediumPurple2");
  table->setData(row++, column, "MediumPurple3");
  table->setData(row++, column, "MediumPurple4");
  table->setData(row++, column, "dark orchid", "white");
  table->setData(row++, column, "DarkOrchid1", "white");
  table->setData(row++, column, "DarkOrchid2", "white");
  table->setData(row++, column, "DarkOrchid3", "white");
  table->setData(row++, column, "DarkOrchid4", "white");
  table->setData(row++, column, "dark violet", "white");
  table->setData(row++, column, "ivory");
  table->setData(row++, column, "ivory2");
  table->setData(row++, column, "ivory3");
  table->setData(row++, column, "ivory4");
  table->setData(row++, column, "beige");
  table->setData(row++, column, "lightyellow");
  table->setData(row++, column, "LightYellow2");
  table->setData(row++, column, "LightYellow3");
  table->setData(row++, column, "LightYellow4");
  table->setData(row++, column, "pale goldenrod");
  table->setData(row++, column, "cornsilk");
  table->setData(row++, column, "cornsilk2");
  table->setData(row++, column, "cornsilk3");
  table->setData(row++, column, "cornsilk4");
  table->setData(row++, column, "lemonchiffon");
  table->setData(row++, column, "LemonChiffon2");
  table->setData(row++, column, "LemonChiffon3");
  table->setData(row++, column, "LemonChiffon4");
  table->setData(row++, column, "light goldenrod yellow");
  table->setData(row++, column, "moccasin");
  table->setData(row++, column, "wheat");
  table->setData(row++, column, "khaki");
  table->setData(row++, column, "khaki2");
  table->setData(row++, column, "khaki3");
  table->setData(row++, column, "khaki4");
  table->setData(row++, column, "dark khaki");
  table->setData(row++, column, "gold");
  table->setData(row++, column, "gold2");
  table->setData(row++, column, "gold3");
  table->setData(row++, column, "gold4");
  table->setData(row++, column, "goldenrod");
  table->setData(row++, column, "goldenrod");
  table->setData(row++, column, "goldenrod2");
  table->setData(row++, column, "goldenrod3");
  table->setData(row++, column, "goldenrod4");
  table->setData(row++, column, "dark goldenrod");
  table->setData(row++, column, "yellow");
  table->setData(row++, column, "yellow2");
  table->setData(row++, column, "yellow3");
  table->setData(row++, column, "yellow4");
  table->setData(row++, column, "orange");
  table->setData(row++, column, "orange2");
  table->setData(row++, column, "orange3");
  table->setData(row++, column, "orange4");
  table->setData(row++, column, "dark orange");
  table->setData(row++, column, "DarkOrange1");
  table->setData(row++, column, "DarkOrange2");
  table->setData(row++, column, "DarkOrange3");
  table->setData(row++, column, "DarkOrange4");
  table->setData(row++, column, "orange red", "white");
  table->setData(row++, column, "OrangeRed2", "white");
  table->setData(row++, column, "OrangeRed3", "white");
  table->setData(row++, column, "OrangeRed4", "white");
  table->setData(row++, column, "floral white");
  table->setData(row++, column, "oldlace");
  table->setData(row++, column, "antique white");
  table->setData(row++, column, "AntiqueWhite1");
  table->setData(row++, column, "AntiqueWhite2");
  table->setData(row++, column, "AntiqueWhite3");
  table->setData(row++, column, "AntiqueWhite4");
  table->setData(row++, column, "peachpuff");
  table->setData(row++, column, "peachpuff");
  table->setData(row++, column, "peachpuff");
  table->setData(row++, column, "peachpuff");
  table->setData(row++, column, "peachpuff");
  table->setData(row++, column, "bisque");
  table->setData(row++, column, "bisque2");
  table->setData(row++, column, "bisque3");
  table->setData(row++, column, "bisque4");
  table->setData(row++, column, "navajowhite");
  table->setData(row++, column, "NavajoWhite2");
  table->setData(row++, column, "NavajoWhite3");
  table->setData(row++, column, "NavajoWhite4");
  table->setData(row++, column, "burlywood");
  table->setData(row++, column, "burlywood2");
  table->setData(row++, column, "burlywood3");
  table->setData(row++, column, "burlywood4");
  table->setData(row++, column, "tan2");
  table->setData(row++, column, "tan3");
  table->setData(row++, column, "tan4");
  table->setData(row++, column, "sandy brown");
  table->setData(row++, column, "sienna", "white");
  table->setData(row++, column, "sienna2", "white");
  table->setData(row++, column, "sienna3", "white");
  table->setData(row++, column, "sienna4", "white");
  table->setData(row++, column, "chocolate", "white");
  table->setData(row++, column, "chocolate2", "white");
  table->setData(row++, column, "chocolate3", "white");
  table->setData(row++, column, "chocolate4", "white");
  table->setData(row++, column, "saddle brown", "white");
  table->setData(row++, column, "coral");
  table->setData(row++, column, "coral2");
  table->setData(row++, column, "coral3");
  table->setData(row++, column, "coral4");
  table->setData(row++, column, "light coral");
  table->setData(row++, column, "rosy brown");
  table->setData(row++, column, "RosyBrown1");
  table->setData(row++, column, "RosyBrown2");
  table->setData(row++, column, "RosyBrown3");
  table->setData(row++, column, "RosyBrown4");
  table->setData(row++, column, "brown", "white");
  table->setData(row++, column, "brown2", "white");
  table->setData(row++, column, "brown3", "white");
  table->setData(row++, column, "brown4", "white");
  table->setData(row++, column, "firebrick", "white");
  table->setData(row++, column, "firebrick2", "white");
  table->setData(row++, column, "firebrick3", "white");
  table->setData(row++, column, "firebrick4", "white");
  table->setData(row++, column, "dark red", "white");
  table->setData(row++, column, "maroon", "white");
  table->setData(row++, column, "maroon2", "white");
  table->setData(row++, column, "maroon3", "white");
  table->setData(row++, column, "maroon4", "white");

  return frame;
}

QFrame*
ExtendedColorDialog::initX11MonoFrame()
{
  auto frame = new ColorDragFrame(this);
  auto layout = new QGridLayout;
  frame->setLayout(layout);

  auto table = createColorTable(frame);
  layout->addWidget(createBtnBox(), 2, 0);

  auto row = 0;
  auto column = 0;
  table->setData(row++, column, "grey0", "white");
  table->setData(row++, column, "grey1", "white");
  table->setData(row++, column, "grey2", "white");
  table->setData(row++, column, "grey3", "white");
  table->setData(row++, column, "grey4", "white");
  table->setData(row++, column, "grey5", "white");
  table->setData(row++, column, "grey6", "white");
  table->setData(row++, column, "grey7", "white");
  table->setData(row++, column, "grey8", "white");
  table->setData(row++, column, "grey9", "white");
  table->setData(row++, column, "grey10", "white");
  table->setData(row++, column, "grey11", "white");
  table->setData(row++, column, "grey12", "white");
  table->setData(row++, column, "grey13", "white");
  table->setData(row++, column, "grey14", "white");
  table->setData(row++, column, "grey15", "white");
  table->setData(row++, column, "grey16", "white");

  row = 0;
  column++;
  table->setData(row++, column, "grey17", "white");
  table->setData(row++, column, "grey18", "white");
  table->setData(row++, column, "grey19", "white");
  table->setData(row++, column, "grey20", "white");
  table->setData(row++, column, "grey21", "white");
  table->setData(row++, column, "grey22", "white");
  table->setData(row++, column, "grey23", "white");
  table->setData(row++, column, "grey24", "white");
  table->setData(row++, column, "grey25", "white");
  table->setData(row++, column, "grey26", "white");
  table->setData(row++, column, "grey27", "white");
  table->setData(row++, column, "grey28", "white");
  table->setData(row++, column, "grey29", "white");
  table->setData(row++, column, "grey30", "white");
  table->setData(row++, column, "grey31", "white");
  table->setData(row++, column, "grey32", "white");
  table->setData(row++, column, "grey33", "white");

  row = 0;
  column++;
  table->setData(row++, column, "grey34", "white");
  table->setData(row++, column, "grey35", "white");
  table->setData(row++, column, "grey36", "white");
  table->setData(row++, column, "grey37", "white");
  table->setData(row++, column, "grey38", "white");
  table->setData(row++, column, "grey39", "white");
  table->setData(row++, column, "grey40");
  table->setData(row++, column, "grey41");
  table->setData(row++, column, "grey42");
  table->setData(row++, column, "grey43");
  table->setData(row++, column, "grey44");
  table->setData(row++, column, "grey45");
  table->setData(row++, column, "grey46");
  table->setData(row++, column, "grey47");
  table->setData(row++, column, "grey48");
  table->setData(row++, column, "grey49");
  table->setData(row++, column, "grey50");

  row = 0;
  column++;
  table->setData(row++, column, "grey51");
  table->setData(row++, column, "grey52");
  table->setData(row++, column, "grey53");
  table->setData(row++, column, "grey54");
  table->setData(row++, column, "grey55");
  table->setData(row++, column, "grey56");
  table->setData(row++, column, "grey57");
  table->setData(row++, column, "grey58");
  table->setData(row++, column, "grey59");
  table->setData(row++, column, "grey60");
  table->setData(row++, column, "grey61");
  table->setData(row++, column, "grey62");
  table->setData(row++, column, "grey63");
  table->setData(row++, column, "grey64");
  table->setData(row++, column, "grey65");
  table->setData(row++, column, "grey66");
  table->setData(row++, column, "grey67");

  row = 0;
  column++;
  table->setData(row++, column, "grey68");
  table->setData(row++, column, "grey69");
  table->setData(row++, column, "grey70");
  table->setData(row++, column, "grey71");
  table->setData(row++, column, "grey72");
  table->setData(row++, column, "grey73");
  table->setData(row++, column, "grey74");
  table->setData(row++, column, "grey75");
  table->setData(row++, column, "grey76");
  table->setData(row++, column, "grey77");
  table->setData(row++, column, "grey78");
  table->setData(row++, column, "grey79");
  table->setData(row++, column, "grey80");
  table->setData(row++, column, "grey81");
  table->setData(row++, column, "grey82");
  table->setData(row++, column, "grey83");
  table->setData(row++, column, "grey84");

  row = 0;
  column++;
  table->setData(row++, column, "grey85");
  table->setData(row++, column, "grey86");
  table->setData(row++, column, "grey87");
  table->setData(row++, column, "grey88");
  table->setData(row++, column, "grey89");
  table->setData(row++, column, "grey90");
  table->setData(row++, column, "grey91");
  table->setData(row++, column, "grey92");
  table->setData(row++, column, "grey93");
  table->setData(row++, column, "grey94");
  table->setData(row++, column, "grey95");
  table->setData(row++, column, "grey96");
  table->setData(row++, column, "grey97");
  table->setData(row++, column, "grey98");
  table->setData(row++, column, "grey99");
  table->setData(row++, column, "grey100");

  row = 0;
  column++;
  table->setData(row++, column, "white");
  table->setData(row++, column, "snow");
  table->setData(row++, column, "ghost white");
  table->setData(row++, column, "white smoke");
  table->setData(row++, column, "snow2");
  table->setData(row++, column, "snow3");
  table->setData(row++, column, "snow4");
  table->setData(row++, column, "gainsboro");
  table->setData(row++, column, "light gray");
  table->setData(row++, column, "dark gray");
  table->setData(row++, column, "light slate gray", "white");
  table->setData(row++, column, "slate gray", "white");
  table->setData(row++, column, "gray", "white");
  table->setData(row++, column, "dim gray", "white");
  table->setData(row++, column, "black", "white");

  row = 0;
  column++;

  return frame;
}

QColor
ExtendedColorDialog::color() const
{
  return m_color;
}

void
ExtendedColorDialog::setColor(const QColor& color)
{
  m_color = color;
  m_colorDlg->setCurrentColor(color);
  for (int i = 1; i < m_tabs->count(); i++) {
    auto frame = qobject_cast<ColorDragFrame*>(m_tabs->widget(i));
    if (frame) {
      frame->getDisplay()->setCurrentColor(color);
    }
  }
}

QColor
ExtendedColorDialog::secondaryColor()
{
  return m_dropColor;
}

void
ExtendedColorDialog::setSecondaryColor(const QColor& color)
{
  m_dropColor = color;
  m_colorDlg->setCurrentColor(m_color);
  for (int i = 1; i < m_tabs->count(); i++) {
    auto frame = qobject_cast<ColorDragFrame*>(m_tabs->widget(i));
    if (frame) {
      frame->getDisplay()->setSecondaryColor(m_dropColor);
    }
  }
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

ColorDropDisplay::ColorDropDisplay(const QColor& color,
                                   const QColor& dropColor,
                                   QWidget* parent)
  : QFrame(parent)
  , m_color(color)
  , m_dropColor(dropColor)
  , m_colorSet(false)
  , m_dropColorSet(false)
{
  if (color != Qt::white)
    m_colorSet = true;
  if (dropColor != Qt::white)
    m_dropColorSet = true;
  setStyleSheet(DISPLAYBORDER);
  setAcceptDrops(true);

  auto layout = new QHBoxLayout;
  layout->setSpacing(0);
  setLayout(layout);
  m_left = new QLabel(this);
  m_left->setStyleSheet(colorToStyle(m_color, Left));
  layout->addWidget(m_left);
  m_right = new QLabel(this);
  m_right->setStyleSheet(colorToStyle(m_dropColor, Right));
  layout->addWidget(m_right);
}

void
ColorDropDisplay::setCurrentColor(const QColor& color)
{
  m_color = color;
  if (!m_dropColorSet) {
    m_dropColor = color;
    m_right->setStyleSheet(colorToStyle(m_dropColor, Right));
    m_dropColorSet = true;
  }
  m_left->setStyleSheet(colorToStyle(m_color, Left));
  m_colorSet = true;
}

void
ColorDropDisplay::setSecondaryColor(const QColor& color)
{
  m_dropColor = color;
  if (!m_colorSet) {
    // if primary color is not yet set set it to this color.
    m_color = color;
    m_left->setStyleSheet(colorToStyle(m_color, Left));
    m_colorSet = true;
  }
  m_right->setStyleSheet(colorToStyle(m_dropColor, Right));
  m_dropColorSet = true;
}

void
ColorDropDisplay::dragEnterEvent(QDragEnterEvent* event)
{
  if (event->mimeData()->hasColor())
    event->acceptProposedAction();
  else
    event->ignore();
}

void
ColorDropDisplay::dropEvent(QDropEvent* event)
{
  if (event->mimeData()->hasColor()) {
    QColor color = event->mimeData()->colorData().value<QColor>();
    setSecondaryColor(color);
  }
}

QString
ColorDropDisplay::colorToStyle(const QColor& color, Side side)
{
  if (color.isValid()) {
    auto red = QStringLiteral("%1").arg(color.red(), 2, 16, QLatin1Char('0'));
    auto green =
      QStringLiteral("%1").arg(color.green(), 2, 16, QLatin1Char('0'));
    auto blue = QStringLiteral("%1").arg(color.blue(), 2, 16, QLatin1Char('0'));
    auto value = QString("#%1%2%3").arg(red, green, blue);
    if (side == Left) {
      auto val = DISPLAYLABELLEFT.arg(value);
      return val;
    } else {
      auto val = DISPLAYLABELRIGHT.arg(value);
      return val;
    }
  }
  auto val = DISPLAYLABELRIGHT.arg("white");
  return val;
}

QColor
ColorDropDisplay::dropColor() const
{
  return m_dropColor;
}

QColor
ColorDropDisplay::color() const
{
  return m_color;
}

ColorDragTable::ColorDragTable(int rows, int columns, QWidget* parent)
  : QTableView(parent)
{
  m_model = new ColorDragModel(rows, columns);
  setModel(m_model);
  setStyleSheet("QTableView { gridline-color: white; }");
}

QString
ColorDragTable::name(const QModelIndex& index)
{
  auto data = m_model->data(index, Qt::DisplayRole);
  return data.toString();
}

QColor
ColorDragTable::foreground(const QModelIndex& index)
{
  auto data = m_model->data(index, Qt::ForegroundRole);
  return data.value<QColor>();
}

QColor
ColorDragTable::background(const QModelIndex& index)
{
  auto data = m_model->data(index, Qt::BackgroundRole);
  return data.value<QColor>();
}

void
ColorDragTable::setData(int row,
                        int column,
                        const QString& back,
                        const QString& fore)
{
  auto background = QColorConstants::Svg::color(back);
  QColor foreground;
  if (!fore.isEmpty()) {
    foreground = QColorConstants::Svg::color(fore);
  }
  m_model->setColorData(row, column, back, background, foreground);
  auto index = m_model->index(row, column);
  if (index.isValid())
    emit dataChanged(index, index);
}

void
ColorDragTable::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    auto pos = event->pos();
    auto index = indexAt(pos);
    if (index.isValid()) {
      m_dragStartPosition = pos;
      m_color = background(index);
      auto w = columnWidth(index.column());
      auto h = rowHeight(index.row());
      m_size = QSize(w, h);
    } else {
      m_color = QColor();
    }
  }
  QTableView::mousePressEvent(event);
}

void
ColorDragTable::mouseMoveEvent(QMouseEvent* event)
{
  auto mousePos = event->pos();
  if (!(event->buttons() & Qt::LeftButton))
    return;
  if ((mousePos - m_dragStartPosition).manhattanLength() <
      QApplication::startDragDistance())
    return;

  if (m_color.isValid()) {
    auto index = indexAt(mousePos);
    QDrag* drag = new QDrag(this);
    m_pixmap = QPixmap(m_size);
    m_pixmap.fill(m_color);
    drag->setPixmap(m_pixmap);
    m_pixmap = QPixmap();
    QMimeData* mimeData = new QMimeData;
    mimeData->setColorData(m_color);
    drag->setMimeData(mimeData);
    drag->setHotSpot(
      QPoint(drag->pixmap().width() / 2, drag->pixmap().height()));

    drag->exec(Qt::CopyAction);
  }
  // dont want the table to do any selection drag.
  // QTableView::mouseMoveEvent(event);
}

void
ColorDragTable::dragEnterEvent(QDragEnterEvent* event)
{
  event->ignore();
}

void
ColorDragTable::dragMoveEvent(QDragMoveEvent* /*event*/)
{
  // does nothing but it blocks the default qtableview
  // drag operations which we don't want.
}

ColorDropDisplay*
ColorDragTable::label() const
{
  return m_label;
}

void
ColorDragTable::setLabel(ColorDropDisplay* label)
{
  m_label = label;
}

ColorDragModel::ColorDragModel(int rows, int columns)
{
  m_data = new Column[rows];
  for (int row = 0; row < rows; ++row) {
    m_data[row] = new Row[rows];
    for (int column = 0; column < columns; ++column) {
      m_data[row][column] = nullptr;
    }
  }

  m_rows = rows;
  m_columns = columns;
}

ColorDragModel::~ColorDragModel()
{
  for (int row = 0; row < m_rows; ++row) {
    auto rowData = m_data[row];
    for (int column = 0; column < m_columns; ++column) {
      auto data = rowData[column];
      if (data)
        delete data;
    }
    delete[] rowData;
  }
  delete[] m_data;
}

Qt::ItemFlags
ColorDragModel::flags(const QModelIndex& index) const
{
  auto name = data(index, Qt::DisplayRole).toString();
  if (name.isEmpty())
    return Qt::NoItemFlags;
  return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable;
}

int
ColorDragModel::rowCount(const QModelIndex& /*parent*/) const
{
  return m_rows;
}

int
ColorDragModel::columnCount(const QModelIndex& /*parent*/) const
{
  return m_columns;
}

QVariant
ColorDragModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  auto row = index.row();
  auto column = index.column();
  auto data = m_data[row][column];
  if (data) {
    if (role == Qt::DisplayRole) {
      return data->name;
    } else if (role == Qt::ForegroundRole) {
      return m_data[row][column]->fore;
    } else if (role == Qt::BackgroundRole) {
      return m_data[row][column]->back;
    }
  }
  return QVariant();
}

QVariant
ColorDragModel::headerData(int /*section*/,
                           Qt::Orientation /*orientation*/,
                           int /*role*/) const
{
  return QVariant();
}

void
ColorDragModel::setColorData(int row,
                             int column,
                             const QString& name,
                             const QColor& back,
                             const QColor& fore)
{
  if (row >= rowCount() || column >= columnCount()) {
    return;
  }
  auto data = m_data[row][column];
  if (data) {
    data->name = name;
    data->back = back;
    data->fore = fore;
  } else {
    data = new Data(name, fore, back);
    m_data[row][column] = data;
  }
}

QModelIndex
ColorDragModel::index(int row, int column, const QModelIndex& /*parent*/) const
{
  if (row >= 0 && row < rowCount() && column >= 0 && column < columnCount()) {
    if (m_data[row][column]) {
      auto newIndex = createIndex(row, column, m_data[row][column]);
      return newIndex;
    }
  }
  return QModelIndex();
}

QModelIndex
ColorDragModel::parent(const QModelIndex& /*child*/) const
{
  return QModelIndex();
}

ColorDragTable*
ColorDragFrame::table() const
{
  return m_table;
}

void
ColorDragFrame::setTable(ColorDragTable* table)
{
  m_table = table;
}

ColorDropDisplay*
ColorDragFrame::getDisplay() const
{
  return display;
}

void
ColorDragFrame::setDisplay(ColorDropDisplay* value)
{
  display = value;
}

QGridLayout*
ColorDragFrame::layout() const
{
  return qobject_cast<QGridLayout*>(QFrame::layout());
}
