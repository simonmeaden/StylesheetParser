#include "stylesheetedit/extendedcolordialog.h"
#include "stylesheetedit/x11colors.h"

#include <QtDebug>

#include <stylesheetedit/extendedcolordialog.h>

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
}

ExtendedColorDialog::ExtendedColorDialog(const QColor& initialColor,
                                         QWidget* parent)
  : QDialog(parent)
  , m_color(initialColor)
  , m_dropColor(QColor(Qt::white))
{
  initGui();
  setColor(initialColor, "white");
}

QColor
ExtendedColorDialog::getSvgOrX11Color(const QString& initialColor)
{
  auto color = QColorConstants::Svg::color(initialColor);
  if (color.isValid()) {
    return color;
  } else {
    color = QColorConstants::X11::color(initialColor);
    if (color.isValid()) {
      return color;
    } else {
      return Qt::white;
    }
  }
}

ExtendedColorDialog::ExtendedColorDialog(const QString& initialColor,
                                         QWidget* parent)
  : QDialog(parent)
{
  initGui();
  auto color = getSvgOrX11Color(initialColor);
  setColor(color, initialColor);
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

ExtendedColorDialog::ExtendedColorDialog(const QString& initialColor,
                                         const QString& secondaryColor,
                                         QWidget* parent)
{
  initGui();
  auto color = getSvgOrX11Color(initialColor);
  setColor(color, initialColor);
  initGui();
  color = getSvgOrX11Color(secondaryColor);
  if (color == Qt::white) {
    setSecondaryColor(color, "white");
  } else {
    setSecondaryColor(color, initialColor);
  }
}

void
ExtendedColorDialog::initGui()
{
  setWindowTitle(tr("Choose Color"));

  m_tabs = new QTabWidget(this);
  auto layout = new QGridLayout;
  layout->addWidget(m_tabs, 0, 0);
  layout->addWidget(createBtnBox(), 1, 0);
  setLayout(layout);

  m_colorDlg = new QColorDialog(m_color, this);
  m_colorDlg->setOptions(QColorDialog::ShowAlphaChannel |
                         QColorDialog::DontUseNativeDialog |
                         QColorDialog::NoButtons);
  m_tabs->addTab(m_colorDlg, "Color Dialog");
  connect(m_colorDlg,
          &QColorDialog::currentColorChanged,
          this,
          &ExtendedColorDialog::currentColorChanged);

  auto svgColors1 = initSvgFrame1();
  m_tabs->addTab(svgColors1, "SVG Color Names 1");

  auto svgColors2 = initSvgFrame2();
  m_tabs->addTab(svgColors2, "SVG Color Names 2");

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

void
ExtendedColorDialog::colorClicked(const QModelIndex& index)
{
  auto table = qobject_cast<ColorDragTable*>(sender());
  if (index.isValid()) {
    m_color = table->background(index);
    m_name = table->name(index);
    table->setStyleSheet(QString("QTableView { gridline-color: white; }"
                                 "QTableView::item:selected { "
                                 "gridline-color: red; "
                                 "qproperty-currentItemBackground: %1;"
                                 "}")
                           .arg(m_color.name()));
    table->display()->setCurrentColor(m_color, m_name);
    emit colorSelected(m_color);
  }
}

void
ExtendedColorDialog::setCurrentColorsFromNamedTab()
{
  auto widget = m_tabs->widget(int(m_currentTab));
  auto frame = qobject_cast<ColorDragFrame*>(widget);
  auto display = frame->display();
  if (display) {
    m_color = display->color();
    m_name = display->name();
    m_dropColor = display->dropColor();
    m_dropName = display->dropName();
  }
}

void
ExtendedColorDialog::setCurrentColorsToNamedTab(int index)
{
  auto widget = m_tabs->widget(index);
  if (widget) {
    auto frame = qobject_cast<ColorDragFrame*>(widget);
    if (frame) {
      auto display = frame->display();
      if (display) {
        display->setCurrentColor(m_color, m_name);
        display->setSecondaryColor(m_dropColor, m_dropName);
      }
    }
  }
}

void
ExtendedColorDialog::tabChanged(int index)
{
  if (index == ColorDialog) {
    if (m_currentTab != ColorDialog) {
      setCurrentColorsFromNamedTab();
    } else {
      m_color = m_colorDlg->currentColor();
    }
    m_colorDlg->setCurrentColor(m_color);
  } else {
    if (m_currentTab == ColorDialog) {
      m_color = m_colorDlg->currentColor();
    } else {
      setCurrentColorsFromNamedTab();
    }
    setCurrentColorsToNamedTab(index);
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
  auto table = new ColorDragTable(17, 7, this);
  auto display = createColorDisplay();
  frame->setDisplay(display);
  frame->setTable(table);
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
ExtendedColorDialog::initSvgFrame1()
{
  auto frame = new ColorDragFrame(this);
  auto layout = new QGridLayout;
  frame->setLayout(layout);

  auto table = createColorTable(frame);

  int row = 0, column = 0;
  // blues
  table->setData(row++, column, false, "aliceblue");
  table->setData(row++, column, false, "azure");
  table->setData(row++, column, false, "lightcyan");
  table->setData(row++, column, false, "paleturquoise");
  table->setData(row++, column, false, "powderblue");
  table->setData(row++, column, false, "lightblue");
  table->setData(row++, column, false, "lightskyblue");
  table->setData(row++, column, false, "skyblue");
  table->setData(row++, column, false, "cornflowerblue");
  table->setData(row++, column, false, "steelblue");
  table->setData(row++, column, false, "dodgerblue");
  table->setData(row++, column, false, "royalblue");
  table->setData(row++, column, false, "deepskyblue");
  table->setData(row++, column, false, "mediumslateblue");
  table->setData(row++, column, false, "slateblue");
  table->setData(row++, column, false, "indigo", "white");
  table->setData(row++, column, false, "darkslateblue", "white");

  row = 0;
  column++;
  table->setData(row++, column, false, "mediumblue", "white");
  table->setData(row++, column, false, "darkblue", "white");
  table->setData(row++, column, false, "navy", "white");
  table->setData(row++, column, false, "blue", "white");
  table->setData(row++, column, false, "midnightblue", "white");
  table->setData(row++, column, false, "honeydew");
  table->setData(row++, column, false, "limegreen");
  table->setData(row++, column, false, "darkseagreen");
  table->setData(row++, column, false, "palegreen");
  table->setData(row++, column, false, "lightgreen");
  table->setData(row++, column, false, "springgreen");
  table->setData(row++, column, false, "lime");
  table->setData(row++, column, false, "forestgreen", "white");
  table->setData(row++, column, false, "green", "white");
  table->setData(row++, column, false, "darkgreen", "white");
  table->setData(row++, column, false, "aquamarine");
  table->setData(row++, column, false, "mediumaquamarine");

  row = 0;
  column++;
  table->setData(row++, column, false, "mediumseagreen");
  table->setData(row++, column, false, "mediumspringgreen");
  table->setData(row++, column, false, "seagreen");
  table->setData(row++, column, false, "greenyellow");
  table->setData(row++, column, false, "lawngreen");
  table->setData(row++, column, false, "chartreuse");
  table->setData(row++, column, false, "yellowgreen");
  table->setData(row++, column, false, "olivedrab", "white");
  table->setData(row++, column, false, "olive", "white");
  table->setData(row++, column, false, "darkolivegreen", "white");
  table->setData(row++, column, false, "cyan");
  table->setData(row++, column, false, "aqua");
  table->setData(row++, column, false, "turquoise");
  table->setData(row++, column, false, "mediumturquoise");
  table->setData(row++, column, false, "darkturquoise");
  table->setData(row++, column, false, "lightseagreen");
  table->setData(row++, column, false, "cadetblue");

  row = 0;
  column++;
  table->setData(row++, column, false, "darkcyan");
  table->setData(row++, column, false, "teal");
  table->setData(row++, column, false, "snow");
  table->setData(row++, column, false, "white");
  table->setData(row++, column, false, "whitesmoke");
  table->setData(row++, column, false, "mintcream");
  table->setData(row++, column, false, "ghostwhite");
  table->setData(row++, column, false, "gainsboro");
  table->setData(row++, column, false, "lightgray");
  table->setData(row++, column, false, "silver");
  table->setData(row++, column, false, "darkgray");
  table->setData(row++, column, false, "lightslategray", "white");
  table->setData(row++, column, false, "slategray", "white");
  table->setData(row++, column, false, "darkslategray", "white");
  table->setData(row++, column, false, "gray", "white");
  table->setData(row++, column, false, "dimgray", "white");
  table->setData(row++, column, false, "black", "white");

  return frame;
}

QFrame*
ExtendedColorDialog::initSvgFrame2()
{
  auto frame = new ColorDragFrame(this);
  auto layout = new QGridLayout;
  frame->setLayout(layout);

  auto table = createColorTable(frame);

  int row = 0, column = 0;

  table->setData(row++, column, false, "seashell");
  table->setData(row++, column, false, "lavenderblush");
  table->setData(row++, column, false, "mistyrose");
  table->setData(row++, column, false, "lightsalmon");
  table->setData(row++, column, false, "darksalmon");
  table->setData(row++, column, false, "salmon");
  table->setData(row++, column, false, "tomato");
  table->setData(row++, column, false, "red", "white");
  table->setData(row++, column, false, "pink");
  table->setData(row++, column, false, "crimson", "white");
  table->setData(row++, column, false, "indianred", "white");
  table->setData(row++, column, false, "lavender");
  table->setData(row++, column, false, "hotpink");
  table->setData(row++, column, false, "lightpink");
  table->setData(row++, column, false, "palevioletred");
  table->setData(row++, column, false, "fuchsia");
  table->setData(row++, column, false, "magenta");

  row = 0;
  column++;
  table->setData(row++, column, false, "deeppink");
  table->setData(row++, column, false, "mediumvioletred");
  table->setData(row++, column, false, "thistle");
  table->setData(row++, column, false, "plum");
  table->setData(row++, column, false, "violet");
  table->setData(row++, column, false, "orchid");
  table->setData(row++, column, false, "mediumorchid");
  table->setData(row++, column, false, "darkmagenta", "white");
  table->setData(row++, column, false, "purple", "white");
  table->setData(row++, column, false, "mediumpurple");
  table->setData(row++, column, false, "darkorchid", "white");
  table->setData(row++, column, false, "darkviolet", "white");
  table->setData(row++, column, false, "blueviolet", "white");
  table->setData(row++, column, false, "ivory");
  table->setData(row++, column, false, "beige");
  table->setData(row++, column, false, "lightyellow");
  table->setData(row++, column, false, "linen");

  row = 0;
  column++;
  table->setData(row++, column, false, "palegoldenrod");
  table->setData(row++, column, false, "cornsilk");
  table->setData(row++, column, false, "floralwhite");
  table->setData(row++, column, false, "lemonchiffon");
  table->setData(row++, column, false, "lightgoldenrodyellow");
  table->setData(row++, column, false, "moccasin");
  table->setData(row++, column, false, "wheat");
  table->setData(row++, column, false, "khaki");
  table->setData(row++, column, false, "darkkhaki");
  table->setData(row++, column, false, "gold");
  table->setData(row++, column, false, "goldenrod");
  table->setData(row++, column, false, "darkgoldenrod");
  table->setData(row++, column, false, "yellow");
  table->setData(row++, column, false, "orange");
  table->setData(row++, column, false, "darkorange");
  table->setData(row++, column, false, "orangered", "white");
  table->setData(row++, column, false, "oldlace");

  row = 0;
  column++;
  table->setData(row++, column, false, "antiquewhite");
  table->setData(row++, column, false, "papayawhip");
  table->setData(row++, column, false, "peachpuff");
  table->setData(row++, column, false, "bisque");
  table->setData(row++, column, false, "navajowhite");
  table->setData(row++, column, false, "blanchedalmond");
  table->setData(row++, column, false, "burlywood");
  table->setData(row++, column, false, "tan");
  table->setData(row++, column, false, "sandybrown");
  table->setData(row++, column, false, "peru", "white");
  table->setData(row++, column, false, "sienna", "white");
  table->setData(row++, column, false, "chocolate", "white");
  table->setData(row++, column, false, "saddlebrown", "white");
  table->setData(row++, column, false, "coral");
  table->setData(row++, column, false, "lightcoral");
  table->setData(row++, column, false, "rosybrown");
  table->setData(row++, column, false, "brown", "white");

  row = 0;
  column++;
  table->setData(row++, column, false, "firebrick", "white");
  table->setData(row++, column, false, "darkred", "white");
  table->setData(row++, column, false, "maroon", "white");

  return frame;
}

QFrame*
ExtendedColorDialog::initX11ColorFrame1()
{
  auto frame = new ColorDragFrame(this);
  auto layout = new QGridLayout;
  frame->setLayout(layout);

  auto table = createColorTable(frame);

  int row = 0, column = 0;
  table->setData(row++, column, true, "mint cream");
  table->setData(row++, column, true, "mint cream");
  table->setData(row++, column, true, "alice blue");
  table->setData(row++, column, true, "azure");
  table->setData(row++, column, true, "azure2");
  table->setData(row++, column, true, "azure3");
  table->setData(row++, column, true, "azure4");
  table->setData(row++, column, true, "light cyan");
  table->setData(row++, column, true, "pale turquoise");
  table->setData(row++, column, true, "PaleTurquoise1");
  table->setData(row++, column, true, "PaleTurquoise2");
  table->setData(row++, column, true, "PaleTurquoise3");
  table->setData(row++, column, true, "PaleTurquoise4");
  table->setData(row++, column, true, "powder blue");
  table->setData(row++, column, true, "cornflower blue");
  table->setData(row++, column, true, "deep sky blue");
  table->setData(row++, column, true, "DeepSkyBlue2");
  table->setData(row++, column, true, "DeepSkyBlue3");
  table->setData(row++, column, true, "DeepSkyBlue4");

  row = 0;
  column++;
  table->setData(row++, column, true, "light blue");
  table->setData(row++, column, true, "LightBlue1");
  table->setData(row++, column, true, "LightBlue2");
  table->setData(row++, column, true, "LightBlue3");
  table->setData(row++, column, true, "LightBlue4");
  table->setData(row++, column, true, "light steel blue");
  table->setData(row++, column, true, "LightSteelBlue1");
  table->setData(row++, column, true, "LightSteelBlue2");
  table->setData(row++, column, true, "LightSteelBlue3");
  table->setData(row++, column, true, "LightSteelBlue4");
  table->setData(row++, column, true, "dark slate gray", "white");
  table->setData(row++, column, true, "DarkSlateGray1", "white");
  table->setData(row++, column, true, "DarkSlateGray2", "white");
  table->setData(row++, column, true, "DarkSlateGray3", "white");
  table->setData(row++, column, true, "DarkSlateGray4", "white");

  row = 0;
  column++;
  table->setData(row++, column, true, "light sky blue");
  table->setData(row++, column, true, "LightSkyBlue1");
  table->setData(row++, column, true, "LightSkyBlue2");
  table->setData(row++, column, true, "LightSkyBlue3");
  table->setData(row++, column, true, "LightSkyBlue4");
  table->setData(row++, column, true, "sky blue");
  table->setData(row++, column, true, "SkyBlue2");
  table->setData(row++, column, true, "SkyBlue3");
  table->setData(row++, column, true, "SkyBlue4");
  table->setData(row++, column, true, "SteelBlue1");
  table->setData(row++, column, true, "SteelBlue2");
  table->setData(row++, column, true, "SteelBlue3");
  table->setData(row++, column, true, "steel blue");
  table->setData(row++, column, true, "SteelBlue4");

  row = 0;
  column++;
  table->setData(row++, column, true, "royal blue");
  table->setData(row++, column, true, "RoyalBlue1");
  table->setData(row++, column, true, "RoyalBlue2");
  table->setData(row++, column, true, "RoyalBlue3");
  table->setData(row++, column, true, "RoyalBlue4");
  table->setData(row++, column, true, "blue", "white");
  table->setData(row++, column, true, "medium blue", "white");
  table->setData(row++, column, true, "dark blue", "white");
  table->setData(row++, column, true, "navy", "white");
  table->setData(row++, column, true, "midnight blue", "white");
  table->setData(row++, column, true, "medium slate blue");
  table->setData(row++, column, true, "slate blue");
  table->setData(row++, column, true, "dark slate blue", "white");
  table->setData(row++, column, true, "dodger blue");
  table->setData(row++, column, true, "DodgerBlue2");
  table->setData(row++, column, true, "DodgerBlue3");
  table->setData(row++, column, true, "DodgerBlue4");

  row = 0;
  column++;
  table->setData(row++, column, true, "lime green");
  table->setData(row++, column, true, "dark sea green");
  table->setData(row++, column, true, "pale green");
  table->setData(row++, column, true, "light green");
  table->setData(row++, column, true, "spring green");
  table->setData(row++, column, true, "forest green", "white");
  table->setData(row++, column, true, "green", "white");
  table->setData(row++, column, true, "dark green", "white");
  table->setData(row++, column, true, "medium sea green");
  table->setData(row++, column, true, "sea green");
  table->setData(row++, column, true, "aquamarine");
  table->setData(row++, column, true, "aquamarine2");

  row = 0;
  column++;
  table->setData(row++, column, true, "aquamarine3");
  table->setData(row++, column, true, "aquamarine4");
  table->setData(row++, column, true, "medium aquamarine");
  table->setData(row++, column, true, "medium spring green");
  table->setData(row++, column, true, "green yellow");
  table->setData(row++, column, true, "lawn green");
  table->setData(row++, column, true, "chartreuse");
  table->setData(row++, column, true, "chartreuse2");
  table->setData(row++, column, true, "chartreuse3");
  table->setData(row++, column, true, "chartreuse4");
  table->setData(row++, column, true, "yellow green");
  table->setData(row++, column, true, "olive drab", "white");
  table->setData(row++, column, true, "dark olive green", "white");

  return frame;
}

QFrame*
ExtendedColorDialog::initX11ColorFrame2()
{
  auto frame = new ColorDragFrame(this);
  auto layout = new QGridLayout;
  frame->setLayout(layout);

  auto table = createColorTable(frame);

  int row = 0, column = 0;
  table->setData(row++, column, true, "cyan");
  table->setData(row++, column, true, "cyan2");
  table->setData(row++, column, true, "cyan3");
  table->setData(row++, column, true, "cyan4");
  table->setData(row++, column, true, "turquoise");
  table->setData(row++, column, true, "turquoise1");
  table->setData(row++, column, true, "turquoise2");
  table->setData(row++, column, true, "turquoise3");
  table->setData(row++, column, true, "turquoise4");
  table->setData(row++, column, true, "dark turquoise");
  table->setData(row++, column, true, "light sea green");
  table->setData(row++, column, true, "dark cyan");
  table->setData(row++, column, true, "seashell1");
  table->setData(row++, column, true, "seashell2");
  table->setData(row++, column, true, "seashell3");
  table->setData(row++, column, true, "seashell4");
  table->setData(row++, column, true, "lavender");

  row = 0;
  column++;
  table->setData(row++, column, true, "LavenderBlush1");
  table->setData(row++, column, true, "LavenderBlush2");
  table->setData(row++, column, true, "LavenderBlush3");
  table->setData(row++, column, true, "LavenderBlush4");
  table->setData(row++, column, true, "misty rose");
  table->setData(row++, column, true, "MistyRose2");
  table->setData(row++, column, true, "MistyRose3");
  table->setData(row++, column, true, "MistyRose4");
  table->setData(row++, column, true, "LightSalmon1");
  table->setData(row++, column, true, "LightSalmon2");
  table->setData(row++, column, true, "LightSalmon3");
  table->setData(row++, column, true, "LightSalmon4");
  table->setData(row++, column, true, "dark salmon");
  table->setData(row++, column, true, "salmon1");
  table->setData(row++, column, true, "salmon2");
  table->setData(row++, column, true, "salmon3");
  table->setData(row++, column, true, "salmon4");
  table->setData(row++, column, true, "tomato1");
  table->setData(row++, column, true, "tomato2");
  table->setData(row++, column, true, "tomato3");

  row = 0;
  column++;
  table->setData(row++, column, true, "tomato4");
  table->setData(row++, column, true, "red1", "white");
  table->setData(row++, column, true, "red2", "white");
  table->setData(row++, column, true, "red3", "white");
  table->setData(row++, column, true, "red4", "white");
  table->setData(row++, column, true, "pink1");
  table->setData(row++, column, true, "pink2");
  table->setData(row++, column, true, "pink3");
  table->setData(row++, column, true, "pink4");
  table->setData(row++, column, true, "indian red", "white");
  table->setData(row++, column, true, "IndianRed1", "white");
  table->setData(row++, column, true, "IndianRed2", "white");
  table->setData(row++, column, true, "IndianRed3", "white");
  table->setData(row++, column, true, "IndianRed4", "white");
  table->setData(row++, column, true, "hot pink");
  table->setData(row++, column, true, "HotPink1");
  table->setData(row++, column, true, "HotPink2");
  table->setData(row++, column, true, "HotPink3");
  table->setData(row++, column, true, "HotPink4");
  table->setData(row++, column, true, "deep pink");

  row = 0;
  column++;
  table->setData(row++, column, true, "DeepPink2");
  table->setData(row++, column, true, "DeepPink3");
  table->setData(row++, column, true, "DeepPink4");
  table->setData(row++, column, true, "light pink");
  table->setData(row++, column, true, "LightPink1");
  table->setData(row++, column, true, "LightPink2");
  table->setData(row++, column, true, "LightPink3");
  table->setData(row++, column, true, "LightPink4");
  table->setData(row++, column, true, "pale violet red");
  table->setData(row++, column, true, "PaleVioletRed1");
  table->setData(row++, column, true, "PaleVioletRed2");
  table->setData(row++, column, true, "PaleVioletRed3");
  table->setData(row++, column, true, "PaleVioletRed4");
  table->setData(row++, column, true, "magenta");
  table->setData(row++, column, true, "magenta2");
  table->setData(row++, column, true, "magenta3");
  table->setData(row++, column, true, "magenta4");
  table->setData(row++, column, true, "medium violet red");
  table->setData(row++, column, true, "thistle");
  table->setData(row++, column, true, "thistle2");

  row = 0;
  column++;
  table->setData(row++, column, true, "thistle3");
  table->setData(row++, column, true, "thistle4");
  table->setData(row++, column, true, "plum");
  table->setData(row++, column, true, "plum2");
  table->setData(row++, column, true, "plum3");
  table->setData(row++, column, true, "plum4");
  table->setData(row++, column, true, "violet");
  table->setData(row++, column, true, "orchid");
  table->setData(row++, column, true, "orchid2");
  table->setData(row++, column, true, "orchid3");
  table->setData(row++, column, true, "orchid4");
  table->setData(row++, column, true, "medium orchid");
  table->setData(row++, column, true, "MediumOrchid1");
  table->setData(row++, column, true, "MediumOrchid2");
  table->setData(row++, column, true, "MediumOrchid3");
  table->setData(row++, column, true, "MediumOrchid4");
  table->setData(row++, column, true, "dark magenta", "white");
  table->setData(row++, column, true, "purple", "white");
  table->setData(row++, column, true, "purple2", "white");
  table->setData(row++, column, true, "purple3", "white");
  table->setData(row++, column, true, "purple4", "white");
  table->setData(row++, column, true, "medium purple");
  table->setData(row++, column, true, "MediumPurple1");
  table->setData(row++, column, true, "MediumPurple2");
  table->setData(row++, column, true, "MediumPurple3");
  table->setData(row++, column, true, "MediumPurple4");
  table->setData(row++, column, true, "dark orchid", "white");
  table->setData(row++, column, true, "DarkOrchid1", "white");
  table->setData(row++, column, true, "DarkOrchid2", "white");
  table->setData(row++, column, true, "DarkOrchid3", "white");
  table->setData(row++, column, true, "DarkOrchid4", "white");
  table->setData(row++, column, true, "dark violet", "white");
  table->setData(row++, column, true, "ivory");
  table->setData(row++, column, true, "ivory2");
  table->setData(row++, column, true, "ivory3");
  table->setData(row++, column, true, "ivory4");
  table->setData(row++, column, true, "beige");
  table->setData(row++, column, true, "lightyellow");
  table->setData(row++, column, true, "LightYellow2");
  table->setData(row++, column, true, "LightYellow3");
  table->setData(row++, column, true, "LightYellow4");
  table->setData(row++, column, true, "pale goldenrod");
  table->setData(row++, column, true, "cornsilk");
  table->setData(row++, column, true, "cornsilk2");
  table->setData(row++, column, true, "cornsilk3");
  table->setData(row++, column, true, "cornsilk4");
  table->setData(row++, column, true, "lemonchiffon");
  table->setData(row++, column, true, "LemonChiffon2");
  table->setData(row++, column, true, "LemonChiffon3");
  table->setData(row++, column, true, "LemonChiffon4");
  table->setData(row++, column, true, "light goldenrod yellow");
  table->setData(row++, column, true, "moccasin");
  table->setData(row++, column, true, "wheat");
  table->setData(row++, column, true, "khaki");
  table->setData(row++, column, true, "khaki2");
  table->setData(row++, column, true, "khaki3");
  table->setData(row++, column, true, "khaki4");
  table->setData(row++, column, true, "dark khaki");
  table->setData(row++, column, true, "gold");
  table->setData(row++, column, true, "gold2");
  table->setData(row++, column, true, "gold3");
  table->setData(row++, column, true, "gold4");
  table->setData(row++, column, true, "goldenrod");
  table->setData(row++, column, true, "goldenrod");
  table->setData(row++, column, true, "goldenrod2");
  table->setData(row++, column, true, "goldenrod3");
  table->setData(row++, column, true, "goldenrod4");
  table->setData(row++, column, true, "dark goldenrod");
  table->setData(row++, column, true, "yellow");
  table->setData(row++, column, true, "yellow2");
  table->setData(row++, column, true, "yellow3");
  table->setData(row++, column, true, "yellow4");
  table->setData(row++, column, true, "orange");
  table->setData(row++, column, true, "orange2");
  table->setData(row++, column, true, "orange3");
  table->setData(row++, column, true, "orange4");
  table->setData(row++, column, true, "dark orange");
  table->setData(row++, column, true, "DarkOrange1");
  table->setData(row++, column, true, "DarkOrange2");
  table->setData(row++, column, true, "DarkOrange3");
  table->setData(row++, column, true, "DarkOrange4");
  table->setData(row++, column, true, "orange red", "white");
  table->setData(row++, column, true, "OrangeRed2", "white");
  table->setData(row++, column, true, "OrangeRed3", "white");
  table->setData(row++, column, true, "OrangeRed4", "white");
  table->setData(row++, column, true, "floral white");
  table->setData(row++, column, true, "oldlace");
  table->setData(row++, column, true, "antique white");
  table->setData(row++, column, true, "AntiqueWhite1");
  table->setData(row++, column, true, "AntiqueWhite2");
  table->setData(row++, column, true, "AntiqueWhite3");
  table->setData(row++, column, true, "AntiqueWhite4");
  table->setData(row++, column, true, "peachpuff");
  table->setData(row++, column, true, "peachpuff");
  table->setData(row++, column, true, "peachpuff");
  table->setData(row++, column, true, "peachpuff");
  table->setData(row++, column, true, "peachpuff");
  table->setData(row++, column, true, "bisque");
  table->setData(row++, column, true, "bisque2");
  table->setData(row++, column, true, "bisque3");
  table->setData(row++, column, true, "bisque4");
  table->setData(row++, column, true, "navajowhite");
  table->setData(row++, column, true, "NavajoWhite2");
  table->setData(row++, column, true, "NavajoWhite3");
  table->setData(row++, column, true, "NavajoWhite4");
  table->setData(row++, column, true, "burlywood");
  table->setData(row++, column, true, "burlywood2");
  table->setData(row++, column, true, "burlywood3");
  table->setData(row++, column, true, "burlywood4");
  table->setData(row++, column, true, "tan2");
  table->setData(row++, column, true, "tan3");
  table->setData(row++, column, true, "tan4");
  table->setData(row++, column, true, "sandy brown");
  table->setData(row++, column, true, "sienna", "white");
  table->setData(row++, column, true, "sienna2", "white");
  table->setData(row++, column, true, "sienna3", "white");
  table->setData(row++, column, true, "sienna4", "white");
  table->setData(row++, column, true, "chocolate", "white");
  table->setData(row++, column, true, "chocolate2", "white");
  table->setData(row++, column, true, "chocolate3", "white");
  table->setData(row++, column, true, "chocolate4", "white");
  table->setData(row++, column, true, "saddle brown", "white");
  table->setData(row++, column, true, "coral");
  table->setData(row++, column, true, "coral2");
  table->setData(row++, column, true, "coral3");
  table->setData(row++, column, true, "coral4");
  table->setData(row++, column, true, "light coral");
  table->setData(row++, column, true, "rosy brown");
  table->setData(row++, column, true, "RosyBrown1");
  table->setData(row++, column, true, "RosyBrown2");
  table->setData(row++, column, true, "RosyBrown3");
  table->setData(row++, column, true, "RosyBrown4");
  table->setData(row++, column, true, "brown", "white");
  table->setData(row++, column, true, "brown2", "white");
  table->setData(row++, column, true, "brown3", "white");
  table->setData(row++, column, true, "brown4", "white");
  table->setData(row++, column, true, "firebrick", "white");
  table->setData(row++, column, true, "firebrick2", "white");
  table->setData(row++, column, true, "firebrick3", "white");
  table->setData(row++, column, true, "firebrick4", "white");
  table->setData(row++, column, true, "dark red", "white");
  table->setData(row++, column, true, "maroon", "white");
  table->setData(row++, column, true, "maroon2", "white");
  table->setData(row++, column, true, "maroon3", "white");
  table->setData(row++, column, true, "maroon4", "white");

  return frame;
}

QFrame*
ExtendedColorDialog::initX11MonoFrame()
{
  auto frame = new ColorDragFrame(this);
  auto layout = new QGridLayout;
  frame->setLayout(layout);

  auto table = createColorTable(frame);

  auto row = 0;
  auto column = 0;
  table->setData(row++, column, true, "grey0", "white");
  table->setData(row++, column, true, "grey1", "white");
  table->setData(row++, column, true, "grey2", "white");
  table->setData(row++, column, true, "grey3", "white");
  table->setData(row++, column, true, "grey4", "white");
  table->setData(row++, column, true, "grey5", "white");
  table->setData(row++, column, true, "grey6", "white");
  table->setData(row++, column, true, "grey7", "white");
  table->setData(row++, column, true, "grey8", "white");
  table->setData(row++, column, true, "grey9", "white");
  table->setData(row++, column, true, "grey10", "white");
  table->setData(row++, column, true, "grey11", "white");
  table->setData(row++, column, true, "grey12", "white");
  table->setData(row++, column, true, "grey13", "white");
  table->setData(row++, column, true, "grey14", "white");
  table->setData(row++, column, true, "grey15", "white");
  table->setData(row++, column, true, "grey16", "white");

  row = 0;
  column++;
  table->setData(row++, column, true, "grey17", "white");
  table->setData(row++, column, true, "grey18", "white");
  table->setData(row++, column, true, "grey19", "white");
  table->setData(row++, column, true, "grey20", "white");
  table->setData(row++, column, true, "grey21", "white");
  table->setData(row++, column, true, "grey22", "white");
  table->setData(row++, column, true, "grey23", "white");
  table->setData(row++, column, true, "grey24", "white");
  table->setData(row++, column, true, "grey25", "white");
  table->setData(row++, column, true, "grey26", "white");
  table->setData(row++, column, true, "grey27", "white");
  table->setData(row++, column, true, "grey28", "white");
  table->setData(row++, column, true, "grey29", "white");
  table->setData(row++, column, true, "grey30", "white");
  table->setData(row++, column, true, "grey31", "white");
  table->setData(row++, column, true, "grey32", "white");
  table->setData(row++, column, true, "grey33", "white");

  row = 0;
  column++;
  table->setData(row++, column, true, "grey34", "white");
  table->setData(row++, column, true, "grey35", "white");
  table->setData(row++, column, true, "grey36", "white");
  table->setData(row++, column, true, "grey37", "white");
  table->setData(row++, column, true, "grey38", "white");
  table->setData(row++, column, true, "grey39", "white");
  table->setData(row++, column, true, "grey40");
  table->setData(row++, column, true, "grey41");
  table->setData(row++, column, true, "grey42");
  table->setData(row++, column, true, "grey43");
  table->setData(row++, column, true, "grey44");
  table->setData(row++, column, true, "grey45");
  table->setData(row++, column, true, "grey46");
  table->setData(row++, column, true, "grey47");
  table->setData(row++, column, true, "grey48");
  table->setData(row++, column, true, "grey49");
  table->setData(row++, column, true, "grey50");

  row = 0;
  column++;
  table->setData(row++, column, true, "grey51");
  table->setData(row++, column, true, "grey52");
  table->setData(row++, column, true, "grey53");
  table->setData(row++, column, true, "grey54");
  table->setData(row++, column, true, "grey55");
  table->setData(row++, column, true, "grey56");
  table->setData(row++, column, true, "grey57");
  table->setData(row++, column, true, "grey58");
  table->setData(row++, column, true, "grey59");
  table->setData(row++, column, true, "grey60");
  table->setData(row++, column, true, "grey61");
  table->setData(row++, column, true, "grey62");
  table->setData(row++, column, true, "grey63");
  table->setData(row++, column, true, "grey64");
  table->setData(row++, column, true, "grey65");
  table->setData(row++, column, true, "grey66");
  table->setData(row++, column, true, "grey67");

  row = 0;
  column++;
  table->setData(row++, column, true, "grey68");
  table->setData(row++, column, true, "grey69");
  table->setData(row++, column, true, "grey70");
  table->setData(row++, column, true, "grey71");
  table->setData(row++, column, true, "grey72");
  table->setData(row++, column, true, "grey73");
  table->setData(row++, column, true, "grey74");
  table->setData(row++, column, true, "grey75");
  table->setData(row++, column, true, "grey76");
  table->setData(row++, column, true, "grey77");
  table->setData(row++, column, true, "grey78");
  table->setData(row++, column, true, "grey79");
  table->setData(row++, column, true, "grey80");
  table->setData(row++, column, true, "grey81");
  table->setData(row++, column, true, "grey82");
  table->setData(row++, column, true, "grey83");
  table->setData(row++, column, true, "grey84");

  row = 0;
  column++;
  table->setData(row++, column, true, "grey85");
  table->setData(row++, column, true, "grey86");
  table->setData(row++, column, true, "grey87");
  table->setData(row++, column, true, "grey88");
  table->setData(row++, column, true, "grey89");
  table->setData(row++, column, true, "grey90");
  table->setData(row++, column, true, "grey91");
  table->setData(row++, column, true, "grey92");
  table->setData(row++, column, true, "grey93");
  table->setData(row++, column, true, "grey94");
  table->setData(row++, column, true, "grey95");
  table->setData(row++, column, true, "grey96");
  table->setData(row++, column, true, "grey97");
  table->setData(row++, column, true, "grey98");
  table->setData(row++, column, true, "grey99");
  table->setData(row++, column, true, "grey100");

  row = 0;
  column++;
  table->setData(row++, column, true, "white");
  table->setData(row++, column, true, "snow");
  table->setData(row++, column, true, "ghost white");
  table->setData(row++, column, true, "white smoke");
  table->setData(row++, column, true, "snow2");
  table->setData(row++, column, true, "snow3");
  table->setData(row++, column, true, "snow4");
  table->setData(row++, column, true, "gainsboro");
  table->setData(row++, column, true, "light gray");
  table->setData(row++, column, true, "dark gray");
  table->setData(row++, column, true, "light slate gray", "white");
  table->setData(row++, column, true, "slate gray", "white");
  table->setData(row++, column, true, "gray", "white");
  table->setData(row++, column, true, "dim gray", "white");
  table->setData(row++, column, true, "black", "white");

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
ExtendedColorDialog::setColor(const QColor& color, const QString& name)
{
  m_color = color;
  m_name = name;
  m_colorDlg->setCurrentColor(color);
  for (int i = 1; i < m_tabs->count(); i++) {
    auto frame = qobject_cast<ColorDragFrame*>(m_tabs->widget(i));
    if (frame) {
      frame->display()->setCurrentColor(color, name);
    }
  }
}

QColor
ExtendedColorDialog::secondaryColor()
{
  return m_dropColor;
}

void
ExtendedColorDialog::setSecondaryColor(const QColor& color, const QString& name)
{
  m_dropColor = color;
  m_dropName = name;
  m_colorDlg->setCurrentColor(m_color);
  for (int i = 1; i < m_tabs->count(); i++) {
    auto frame = qobject_cast<ColorDragFrame*>(m_tabs->widget(i));
    if (frame) {
      frame->display()->setSecondaryColor(color, name);
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

QString
ExtendedColorDialog::secondaryName() const
{
  return m_dropName;
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
ColorDropDisplay::setCurrentColor(const QColor& color, const QString& name)
{
  m_color = color;
  m_name = name;
  if (!m_dropColorSet) {
    m_dropColor = color;
    m_dropName = name;
    m_right->setStyleSheet(colorToStyle(m_dropColor, Right));
    m_dropColorSet = true;
  }
  m_left->setStyleSheet(colorToStyle(m_color, Left));
  m_colorSet = true;
}

void
ColorDropDisplay::setSecondaryColor(const QColor& color, const QString& name)
{
  m_dropColor = color;
  m_dropName = name;
  if (!m_colorSet) {
    // if primary color is not yet set set it to this color.
    m_color = color;
    m_name = name;
    m_left->setStyleSheet(colorToStyle(m_color, Left));
    m_colorSet = true;
  }
  m_right->setStyleSheet(colorToStyle(m_dropColor, Right));
  m_dropColorSet = true;
}

void
ColorDropDisplay::dragEnterEvent(QDragEnterEvent* event)
{
  if (event->mimeData()->hasFormat("colordata"))
    event->acceptProposedAction();
  else
    event->ignore();
}

void
ColorDropDisplay::dropEvent(QDropEvent* event)
{
  if (event->mimeData()->hasFormat("colordata")) {
    auto mimeData = event->mimeData();
    if (mimeData) {
      ColorDragData data;
      auto array = mimeData->data("colordata");
      QDataStream ds(&array, QIODevice::ReadWrite);
      ds.setVersion(QDataStream::Qt_5_15);
      ds >> data;
      auto name = QString(data.name);
      auto color = QColor(data.r, data.g, data.b);
      setSecondaryColor(color, name);
    }
  }
}

QString
ColorDropDisplay::colorToStyle(const QColor& color, Side side)
{
  if (color.isValid()) {
    QString value = color.name();
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

QString
ColorDropDisplay::dropName() const
{
  return m_dropName;
}

QString
ColorDropDisplay::name() const
{
  return m_name;
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
  auto name = data.toString();
  return name;
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
                        bool x11,
                        const QString& back,
                        const QString& fore)
{
  QColor background, foreground;
  if (x11) {
    background = QColorConstants::X11::color(back);
  } else {
    background = QColorConstants::Svg::color(back);
  }
  if (!fore.isEmpty()) {
    if (x11) {
      foreground = QColorConstants::X11::color(fore);
    } else {
      foreground = QColorConstants::Svg::color(fore);
    }
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
      m_name = name(index);
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
    auto drag = new QDrag(this);
    m_pixmap = QPixmap(m_size);
    m_pixmap.fill(m_color);
    drag->setPixmap(m_pixmap);

    ColorDragData data;
    data.r = m_color.red();
    data.g = m_color.green();
    data.b = m_color.blue();
    data.name = m_name;
    QByteArray array;
    QDataStream ds(&array, QIODevice::ReadWrite);
    ds.setVersion(QDataStream::Qt_5_15);
    ds << data;

    m_pixmap = QPixmap();
    auto mimeData = new QMimeData();
    mimeData->setData("colordata", array);
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
ColorDragTable::display() const
{
  return m_display;
}

void
ColorDragTable::setLabel(ColorDropDisplay* label)
{
  m_display = label;
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
ColorDragFrame::display() const
{
  return m_display;
}

void
ColorDragFrame::setDisplay(ColorDropDisplay* value)
{
  m_display = value;
}

QGridLayout*
ColorDragFrame::layout() const
{
  return qobject_cast<QGridLayout*>(QFrame::layout());
}

QDataStream&
operator<<(QDataStream& out, const ColorDragData& data)
{
  out << data.r << data.g << data.b << data.name;
  return out;
}

QDataStream&
operator>>(QDataStream& in, ColorDragData& data)
{
  in >> data.r >> data.g >> data.b >> data.name;
  return in;
}
