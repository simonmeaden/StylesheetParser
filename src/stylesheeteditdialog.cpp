/*
  Copyright 2020 Simon Meaden

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
        */
#include "stylesheetparser/stylesheeteditdialog.h"
#include "sm_widgets/abstractlabelledwidget.h"
#include "sm_widgets/labelledtextfield.h"
#include "stylesheetedit_p.h"
#include "stylesheetparser/stylesheetedit.h"

StylesheetEditDialog::StylesheetEditDialog(StylesheetEditor* editor,
                                           QWidget* parent)
  : QDialog(parent)
  , m_editor(editor)
{
  auto mainLayout = new QVBoxLayout;
  setLayout(mainLayout);

  m_tabs = new QTabWidget(this);
  mainLayout->addWidget(m_tabs);

  m_btnBox = new QDialogButtonBox(
    QDialogButtonBox::Apply | QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  connect(m_btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(m_btnBox->button(QDialogButtonBox::Apply),
          &QPushButton::clicked,
          this,
          &StylesheetEditDialog::apply);

  m_tabs->addTab(new ColorFontFrame(m_editor, this), tr("Font & Colors"));

  mainLayout->addWidget(m_btnBox);
}

void
StylesheetEditDialog::setEditor(StylesheetEdit* editor)
{
  m_editor = editor->editor();
}

void
StylesheetEditDialog::apply()
{}

const QString ColorFontFrame::DISPLAY =
  "QTabBar :: tab : selected , QTabBar::tab:hover {\n"
  "  background: qlineargradient(\n"
  "    x1: 0, y1: 0, x2: 0, y2: 1,\n"
  "    stop: 0 #fafafa, stop: 0.4 #f4f4f4,\n"
  "    stop: 0.5 #e7e7e7, stop: 1.0 #fafafa\n"
  "  );\n"
  "}\n"
  "QPushButton#evilButton {\n"
  "  background-color: red;\n"
  "  border-style: outset;\n"
  "  border-width: 2px;\n"
  "  border-radius: 10px;\n"
  "  border-color: beige;\n"
  "  font: bold 14px;\n"
  "  min-width: 10em;\n"
  "  padding: 6px;\n"
  "}\n"
  "QPushButton#evilButton:pressed {\n"
  "  background-color: rgb(224, 0, 0);\n"
  "  border-style: inset;\n"
  "}\n";

ColorFontFrame::ColorFontFrame(StylesheetEditor* editor, QWidget* parent)
  : QFrame(parent)
  , m_editor(editor)
{
  QGridLayout* mainLayout = new QGridLayout;
  setLayout(mainLayout);

  QGroupBox* fontGrp = new QGroupBox(tr("Font"), this);
  mainLayout->addWidget(fontGrp);

  QGridLayout* fontLayout = new QGridLayout;
  fontGrp->setLayout(fontLayout);

  auto familyBox = new LabelledComboBox(tr("Family :"), this);
  QFontDatabase database;
  familyBox->addItems(database.families());
  auto font = m_editor->font();
  familyBox->setCurrentText(font.family());
  fontLayout->addWidget(familyBox, 0, 0);

  QStringList values;
  values << "6"
         << "7"
         << "8"
         << "9"
         << "10"
         << "11"
         << "12"
         << "14"
         << "16"
         << "18"
         << "20"
         << "22"
         << "24"
         << "26"
         << "28"
         << "36"
         << "48"
         << "72";
  auto sizeBox = new LabelledComboBox(tr("Size :"), this);
  sizeBox->addItems(values);
  sizeBox->setCurrentText(QString::number(font.pointSize()));
  fontLayout->addWidget(sizeBox, 0, 1);

  auto zoomBox = new LabelledSpinBox(tr("Zoom :"), this);
  zoomBox->setMaximum(2000);
  zoomBox->setMinimum(10);
  zoomBox->setSingleStep(10);

  fontLayout->addWidget(zoomBox, 0, 2);

  auto aliasBox = new QCheckBox(tr("Antialias"), this);
  fontLayout->addWidget(aliasBox, 1, 0, 1, 3, Qt::AlignLeft);

  auto themeBox = new QGroupBox(tr("Color Scheme for Themd \"Flat\""), this);
  auto themeLayout = new QGridLayout;
  themeBox->setLayout(themeLayout);
  mainLayout->addWidget(themeBox);

  auto typeLayout = new QHBoxLayout;
  values.clear();
  values << "Default"
         << "QtCreator dark"
         << "Dark"
         << "Default Classic"
         << "Grayscale"
         << "Inkpot"
         << "IntelliJ IDEA"
         << "Modnokai Night Shift v2"
         << "Solarized Dark"
         << "Solarized Light";
  // TODO get custom copies.
  auto themes = new QComboBox(this);
  themes->addItems(values);
  typeLayout->addWidget(themes, 1);

  auto copyBtn = new QPushButton(tr("Copy"), this);
  typeLayout->addWidget(copyBtn, 0);
  auto deleteBtn = new QPushButton(tr("delete"), this);
  typeLayout->addWidget(deleteBtn, 0);
  themeLayout->addLayout(typeLayout, 0, 0, 1, 3);

  m_model = new TypeModel(this);
  m_model->setData(m_editor);
  auto typeList = new QListView(this);
  typeList->setModel(m_model);
  connect(typeList, &QListView::clicked, this, &ColorFontFrame::indexChanged);
  themeLayout->addWidget(typeList, 1, 0);

  auto display = new StylesheetEdit(this);
  display->setPlainText(DISPLAY);
  themeLayout->addWidget(display, 1, 1);

  m_modify = new ModifyFrame(this);
  themeLayout->addWidget(m_modify, 1, 2);
}

void
ColorFontFrame::indexChanged(const QModelIndex& index)
{
  auto data = m_model->data(index, Qt::UserRole + 1);
  auto format = data.value<QTextCharFormat>();
  m_modify->setRow(index.row(), format);
}

TypeModel::TypeModel(QObject* parent)
  : QAbstractListModel(parent)
{}

int
TypeModel::rowCount(const QModelIndex&) const
{
  auto s =  m_text.size();
  return s;
}

int
TypeModel::columnCount(const QModelIndex&) const
{
  return 1;
}

QVariant
TypeModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();
  auto row = index.row();
  switch (role) {
    default:
    case Qt::DisplayRole: // QString
      return m_text.at(row);
    case Qt::BackgroundRole: // QBrush
      return m_formats.at(row).background();
    case Qt::ForegroundRole: // QBrush
      return m_formats.at(row).foreground();
    case Qt::FontRole: // QFont
      return m_formats.at(row).font();
    case Qt::UserRole + 1: // QTextCharFormat
      QVariant v;
      v.setValue(m_formats.at(row));
      return v;
  }
  return QVariant();
}

void
TypeModel::setData(StylesheetEditor* editor)
{
  m_editor = editor;
  m_text.clear();
  m_formats.clear();
  auto highlighter = editor->highlighter();

  setData("Widget", highlighter->widgetFormat());
  setData("ID Selector", highlighter->idSelectorFormat());
  setData("ID Selector Marker #", highlighter->idSelectorMarkerFormat());
  setData("Sub Control", highlighter->subControlFormat());
  setData("Sub Control marker ::", highlighter->subControlMarkerFormat());
  setData("Pseudo State", highlighter->pseudoStateFormat());
  setData("Pseudo State Marker :", highlighter->pseudoStateMarkerFormat());
  setData("Property Name", highlighter->propertyFormat());
  setData("Property Name Marker :", highlighter->propertyMarkerFormat());
  setData("Property Value", highlighter->valueFormat());
  setData("Property End Marker ;", highlighter->propertyEndMarkerFormat());
  setData("Start Brace {", highlighter->startBraceFormat());
  setData("End Brace }", highlighter->endBraceFormat());
  setData("Mismatched Paranthesis {}", highlighter->braceMatchFormat());
  setData("/* Comment */", highlighter->commentFormat());
}

void
TypeModel::setData(const QString& text, QTextCharFormat format)
{
  auto font = format.font();
  m_fontfamily = format.fontFamily();
  auto fore = format.foreground();
  auto back = format.background();
  m_text.append(text);
  m_formats.append(format);
}

ModifyFrame::ModifyFrame(QWidget* parent)
  : QWidget(parent)
{
  auto layout = new QVBoxLayout;
  setLayout(layout);

  auto colorLayout = new QFormLayout;
  auto foregroundBtn = new QPushButton(this);
  colorLayout->addRow(tr("Foreground"), foregroundBtn);
  connect(foregroundBtn,
          &QPushButton::clicked,
          this,
          &ModifyFrame::foregroundClicked);
  auto backgroundBtn = new QPushButton(this);
  colorLayout->addRow(tr("Background"), backgroundBtn);
  connect(foregroundBtn,
          &QPushButton::clicked,
          this,
          &ModifyFrame::backgroundClicked);
  layout->addLayout(colorLayout);

  auto fontLayout = new QGridLayout;
  auto lbl = new QLabel("Font");
  auto font = lbl->font();
  font.setBold(true);
  lbl->setFont(font);
  fontLayout->addWidget(lbl, 0, 0);
  auto boldBox = new QCheckBox(tr("Bold"), this);
  connect(boldBox, &QCheckBox::clicked, this, &ModifyFrame::boldClicked);
  fontLayout->addWidget(boldBox, 1, 0);
  auto italicBox = new QCheckBox(tr("Italic"), this);
  connect(italicBox, &QCheckBox::clicked, this, &ModifyFrame::italicClicked);
  fontLayout->addWidget(italicBox, 1, 1);
  layout->addLayout(fontLayout);

  auto underlineLayout = new QGridLayout;
  lbl = new QLabel("Underline");
  lbl->setFont(font);
  underlineLayout->addWidget(lbl, 0, 0);
  lbl = new QLabel("Color");
  underlineLayout->addWidget(lbl, 1, 0);
  auto colorBtn = new QPushButton(this);
  connect(colorBtn, &QPushButton::clicked, this, &ModifyFrame::colorClicked);
  underlineLayout->addWidget(colorBtn, 1, 1);
  QStringList values;
  values << tr("No Underline") << "Single"
         << "Wave"
         << "Dot"
         << "Dash"
         << "Dash-Dot"
         << "Dash-Dot-Dot";
  auto underlineType = new QComboBox(this);
  connect(underlineType,
          qOverload<int>(&QComboBox::currentIndexChanged),
          this,
          &ModifyFrame::underlineTypeChanged);
  underlineLayout->addWidget(underlineType, 2, 0);
  layout->addLayout(underlineLayout);
}

void
ModifyFrame::setRow(int row, QTextCharFormat format)
{
  // TODO
}

void
ModifyFrame::foregroundClicked(bool)
{
  // TODO
  //  auto dlg = new QColorDialog(color, this);
}

void
ModifyFrame::backgroundClicked(bool)
{
  // TODO
}

void
ModifyFrame::colorClicked(bool)
{
  // TODO
}

void
ModifyFrame::boldClicked(bool)
{
  // TODO
}

void
ModifyFrame::italicClicked(bool)
{
  // TODO
}

void
ModifyFrame::underlineTypeChanged(int index)
{
  // TODO
}
