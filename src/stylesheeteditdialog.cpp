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
#include "stylesheetedit/stylesheeteditdialog.h"
#include "sm_widgets/abstractlabelledwidget.h"
#include "sm_widgets/labelledtextfield.h"
#include "stylesheetedit/stylesheetedit.h"
#include "stylesheetedit_p.h"
#include "stylesheethighlighter.h"

StylesheetEditDialog::StylesheetEditDialog(DataStore* datastore,
                                           StylesheetEditor* editor,
                                           QWidget* parent)
  : QDialog(parent)
  , m_datastore(datastore)
  , m_editor(editor)
{
  auto mainLayout = new QVBoxLayout;
  setLayout(mainLayout);

  m_tabs = new QTabWidget(this);
  mainLayout->addWidget(m_tabs);

  m_btnBox = new QDialogButtonBox(
    QDialogButtonBox::Apply | QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
  m_btnBox->button(QDialogButtonBox::Ok)
    ->setToolTip(tr("Accept changes and apply them to the editor"));
  connect(
    m_btnBox, &QDialogButtonBox::accepted, this, &StylesheetEditDialog::accept);
  m_btnBox->button(QDialogButtonBox::Cancel)
    ->setToolTip(tr("Cancel changes, even if previously applied"));
  connect(m_btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  m_btnBox->button(QDialogButtonBox::Apply)
    ->setToolTip(
      tr("Apply changes to editor, click 'Cancel' to reject changes"));
  connect(m_btnBox->button(QDialogButtonBox::Apply),
          &QPushButton::clicked,
          this,
          &StylesheetEditDialog::apply);
  connect(m_btnBox->button(QDialogButtonBox::Apply),
          &QPushButton::clicked,
          this,
          &StylesheetEditDialog::apply);

  m_colorFontFrame = new ColorFontFrame(m_datastore, m_editor, this);
  m_tabs->addTab(m_colorFontFrame, tr("Font & Colors"));

  mainLayout->addWidget(m_btnBox);
}

void
StylesheetEditDialog::setEditor(StylesheetEdit* editor)
{
  m_editor = editor->editor();
}

QList<QTextCharFormat>
StylesheetEditDialog::formats() const
{
  return m_colorFontFrame->formats();
}

void
StylesheetEditDialog::accept()
{
  apply();
  QDialog::accept();
}

void
StylesheetEditDialog::reject()
{
  m_colorFontFrame->resetEditorFormats();
  QDialog::reject();
}

void
StylesheetEditDialog::apply()
{
  m_colorFontFrame->applyEditorFormats();
}

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

void
ColorFontFrame::populateDisplay()
{
  m_display->setWidgetFormat(m_model->format(0));
  m_display->setBadWidgetFormat(m_model->format(1));
  m_display->setSeperatorFormat(m_model->format(2));
  m_display->setIdSelectorFormat(m_model->format(3));
  m_display->setBadIdSelectorFormat(m_model->format(4));
  m_display->setIdSelectorMarkerFormat(m_model->format(5));
  m_display->setBadIdSelectorMarkerFormat(m_model->format(6));
  m_display->setSubControlFormat(m_model->format(7));
  m_display->setBadSubControlFormat(m_model->format(8));
  m_display->setSubControlMarkerFormat(m_model->format(9));
  m_display->setBadSubControlMarkerFormat(m_model->format(10));
  m_display->setPseudoStateFormat(m_model->format(11));
  m_display->setBadPseudoStateFormat(m_model->format(12));
  m_display->setPseudoStateMarkerFormat(m_model->format(13));
  m_display->setBadPseudoStateMarkerFormat(m_model->format(14));
  m_display->setPropertyFormat(m_model->format(15));
  m_display->setBadPropertyFormat(m_model->format(16));
  m_display->setPropertyMarkerFormat(m_model->format(17));
  m_display->setPropertyValueFormat(m_model->format(18));
  m_display->setBadPropertyValueFormat(m_model->format(19));
  m_display->setPropertyEndMarkerFormat(m_model->format(20));
  m_display->setStartBraceFormat(m_model->format(21));
  m_display->setBadStartBraceFormat(m_model->format(22));
  m_display->setEndBraceFormat(m_model->format(23));
  m_display->setBadEndBraceFormat(m_model->format(24));
  m_display->setBraceMatchFormat(m_model->format(25));
  m_display->setBadBraceMatchFormat(m_model->format(26));
  m_display->setCommentFormat(m_model->format(27));
  m_display->setPlainText(DISPLAY);
}

void
ColorFontFrame::copyTheme()
{}

ColorFontFrame::ColorFontFrame(DataStore* datastore,
                               StylesheetEditor* editor,
                               QWidget* parent)
  : QFrame(parent)
  , m_datastore(datastore)
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

  auto sizeBox = new LabelledComboBox(tr("Size :"), this);
  sizeBox->addItems(m_datastore->fontSizes());
  sizeBox->setCurrentText(QString::number(font.pointSize()));
  fontLayout->addWidget(sizeBox, 0, 1);

  auto zoomBox = new LabelledSpinBox(tr("Zoom :"), this);
  zoomBox->setMaximum(2000);
  zoomBox->setMinimum(10);
  zoomBox->setSingleStep(10);

  fontLayout->addWidget(zoomBox, 0, 2);

  auto aliasBox = new QCheckBox(tr("Antialias"), this);
  fontLayout->addWidget(aliasBox, 1, 0, 1, 3, Qt::AlignLeft);

  auto themeBox = new QGroupBox(
    tr("Color Scheme for Theme \"%1\"").arg(m_datastore->qtThemeName()), this);
  auto themeLayout = new QGridLayout;
  themeBox->setLayout(themeLayout);
  mainLayout->addWidget(themeBox);

  auto typeLayout = new QHBoxLayout;
  auto customThemes = m_datastore->customThemeNames();
  auto themes = new QComboBox(this);
  themes->addItems(customThemes);
  themes->setCurrentText(m_datastore->currentThemeName());
  typeLayout->addWidget(themes, 1);

  auto copyBtn = new QPushButton(tr("Copy"), this);
  connect(copyBtn, &QPushButton::clicked, this, &ColorFontFrame::copyTheme);
  typeLayout->addWidget(copyBtn, 0);

  auto deleteBtn = new QPushButton(tr("delete"), this);
  typeLayout->addWidget(deleteBtn, 0);
  themeLayout->addLayout(typeLayout, 0, 0, 1, 3);

  m_editor = editor;
  auto highlighter = editor->highlighter();
  m_model = new TypeModel(this);
  m_model->populate(highlighter);
  m_typeList = new QListView(this);
  m_typeList->setModel(m_model);
  connect(m_typeList, &QListView::clicked, this, &ColorFontFrame::indexChanged);
  themeLayout->addWidget(m_typeList, 1, 0);

  m_display = new StylesheetEditor(m_datastore, this);
  populateDisplay();
  themeLayout->addWidget(m_display, 1, 1);

  m_modify = new ModifyFrame(this);
  themeLayout->addWidget(m_modify, 1, 2);
  connect(
    m_modify, &ModifyFrame::rowChanged, this, &ColorFontFrame::rowChanged);
  m_typeList->setCurrentIndex(m_model->index(0, 0));
  m_modify->setRow(0, m_model->format(0));
}

QList<QTextCharFormat>
ColorFontFrame::formats() const
{
  return m_model->formats();
}

void
ColorFontFrame::indexChanged(const QModelIndex& index)
{
  auto format = m_model->format(index.row());
  m_modify->setRow(index.row(), format);
}

void
ColorFontFrame::updateChangedFormat(int row, QTextCharFormat format)
{
  switch (row) {
    case 0:
      m_display->setWidgetFormat(format);
      break;
    case 1:
      m_display->setBadWidgetFormat(format);
      break;
    case 2:
      m_display->setSeperatorFormat(format);
      break;
    case 3:
      m_display->setIdSelectorFormat(format);
      break;
    case 4:
      m_display->setBadIdSelectorFormat(format);
      break;
    case 5:
      m_display->setIdSelectorMarkerFormat(format);
      break;
    case 6:
      m_display->setBadIdSelectorMarkerFormat(format);
      break;
    case 7:
      m_display->setSubControlFormat(format);
      break;
    case 8:
      m_display->setBadSubControlFormat(format);
      break;
    case 9:
      m_display->setSubControlMarkerFormat(format);
      break;
    case 10:
      m_display->setBadSubControlMarkerFormat(format);
      break;
    case 11:
      m_display->setPseudoStateFormat(format);
      break;
    case 12:
      m_display->setBadPseudoStateFormat(format);
      break;
    case 13:
      m_display->setPseudoStateMarkerFormat(format);
      break;
    case 14:
      m_display->setBadPseudoStateMarkerFormat(format);
      break;
    case 15:
      m_display->setPropertyFormat(format);
      break;
    case 16:
      m_display->setBadPropertyFormat(format);
      break;
    case 17:
      m_display->setPropertyMarkerFormat(format);
      break;
    case 18:
      m_display->setPropertyValueFormat(format);
      break;
    case 19:
      m_display->setBadPropertyValueFormat(format);
      break;
    case 20:
      m_display->setPropertyEndMarkerFormat(format);
      break;
    case 21:
      m_display->setStartBraceFormat(format);
      break;
    case 22:
      m_display->setBadStartBraceFormat(format);
      break;
    case 23:
      m_display->setEndBraceFormat(format);
      break;
    case 24:
      m_display->setBadEndBraceFormat(format);
      break;
    case 25:
      m_display->setBraceMatchFormat(format);
      break;
    case 26:
      m_display->setBadBraceMatchFormat(format);
      break;
    case 27:
      m_display->setCommentFormat(format);
      break;
  }
}

void
ColorFontFrame::applyEditorFormats()
{
  m_editor->setWidgetFormat(m_model->format(0));
  m_editor->setBadWidgetFormat(m_model->format(1));
  m_editor->setSeperatorFormat(m_model->format(2));
  m_editor->setIdSelectorFormat(m_model->format(3));
  m_editor->setBadIdSelectorFormat(m_model->format(4));
  m_editor->setIdSelectorMarkerFormat(m_model->format(5));
  m_editor->setBadIdSelectorMarkerFormat(m_model->format(6));
  m_editor->setSubControlFormat(m_model->format(7));
  m_editor->setBadSubControlFormat(m_model->format(8));
  m_editor->setSubControlMarkerFormat(m_model->format(9));
  m_editor->setBadSubControlMarkerFormat(m_model->format(10));
  m_editor->setPseudoStateFormat(m_model->format(11));
  m_editor->setBadPseudoStateFormat(m_model->format(12));
  m_editor->setPseudoStateMarkerFormat(m_model->format(13));
  m_editor->setBadPseudoStateMarkerFormat(m_model->format(14));
  m_editor->setPropertyFormat(m_model->format(15));
  m_editor->setBadPropertyFormat(m_model->format(16));
  m_editor->setPropertyMarkerFormat(m_model->format(17));
  m_editor->setPropertyValueFormat(m_model->format(18));
  m_editor->setBadPropertyValueFormat(m_model->format(19));
  m_editor->setPropertyEndMarkerFormat(m_model->format(20));
  m_editor->setStartBraceFormat(m_model->format(21));
  m_editor->setBadStartBraceFormat(m_model->format(22));
  m_editor->setEndBraceFormat(m_model->format(23));
  m_editor->setBadEndBraceFormat(m_model->format(24));
  m_editor->setBraceMatchFormat(m_model->format(25));
  m_editor->setBadBraceMatchFormat(m_model->format(26));
  m_editor->setCommentFormat(m_model->format(27));
}

void
ColorFontFrame::resetEditorFormats()
{
  m_editor->setWidgetFormat(m_model->originalFormat(0));
  m_editor->setBadWidgetFormat(m_model->originalFormat(1));
  m_editor->setSeperatorFormat(m_model->originalFormat(2));
  m_editor->setIdSelectorFormat(m_model->originalFormat(3));
  m_editor->setBadIdSelectorFormat(m_model->originalFormat(4));
  m_editor->setIdSelectorMarkerFormat(m_model->originalFormat(5));
  m_editor->setBadIdSelectorMarkerFormat(m_model->originalFormat(6));
  m_editor->setSubControlFormat(m_model->originalFormat(7));
  m_editor->setBadSubControlFormat(m_model->originalFormat(8));
  m_editor->setSubControlMarkerFormat(m_model->originalFormat(9));
  m_editor->setBadSubControlMarkerFormat(m_model->originalFormat(10));
  m_editor->setPseudoStateFormat(m_model->originalFormat(11));
  m_editor->setBadPseudoStateFormat(m_model->originalFormat(12));
  m_editor->setPseudoStateMarkerFormat(m_model->originalFormat(13));
  m_editor->setBadPseudoStateMarkerFormat(m_model->originalFormat(14));
  m_editor->setPropertyFormat(m_model->originalFormat(15));
  m_editor->setBadPropertyFormat(m_model->originalFormat(16));
  m_editor->setPropertyMarkerFormat(m_model->originalFormat(17));
  m_editor->setPropertyValueFormat(m_model->originalFormat(18));
  m_editor->setBadPropertyValueFormat(m_model->originalFormat(19));
  m_editor->setPropertyEndMarkerFormat(m_model->originalFormat(20));
  m_editor->setStartBraceFormat(m_model->originalFormat(21));
  m_editor->setBadStartBraceFormat(m_model->originalFormat(22));
  m_editor->setEndBraceFormat(m_model->originalFormat(23));
  m_editor->setBadEndBraceFormat(m_model->originalFormat(24));
  m_editor->setBraceMatchFormat(m_model->originalFormat(25));
  m_editor->setBadBraceMatchFormat(m_model->originalFormat(26));
  m_editor->setCommentFormat(m_model->originalFormat(27));
}

void
ColorFontFrame::rowChanged(int row, QTextCharFormat format)
{
  m_model->setFormat(row, format);
  updateChangedFormat(row, format);
  m_display->handleRehighlight();
}

TypeModel::TypeModel(QObject* parent)
  : QAbstractListModel(parent)
{}

int
TypeModel::rowCount(const QModelIndex&) const
{
  return m_text.size();
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
      return QVariant();
    case Qt::DisplayRole: // QString
      return m_text.at(row);
    case Qt::BackgroundRole: // QBrush
      return m_formats.at(row).background();
    case Qt::ForegroundRole: // QBrush
      return m_formats.at(row).foreground();
    case Qt::FontRole: // QFont
      return m_formats.at(row).font();
  }
  return QVariant();
}

void
TypeModel::populate(StylesheetHighlighter* highlighter)
{
  m_text.clear();
  m_formats.clear();
  m_originalFormats.clear();
  populateItem("Widget", highlighter->widgetFormat());
  populateItem("Bad Widget", highlighter->badWidgetFormat());
  populateItem("Widget Seperator ','", highlighter->seperatorFormat());
  populateItem("ID Selector", highlighter->idSelectorFormat());
  populateItem("Bad ID Selector", highlighter->badIdSelectorFormat());
  populateItem("ID Selector Marker #", highlighter->idSelectorMarkerFormat());
  populateItem("Bad ID Selector Marker #",
               highlighter->badIdSelectorMarkerFormat());
  populateItem("Sub Control", highlighter->subControlFormat());
  populateItem("Bad Sub Control", highlighter->badSubControlFormat());
  populateItem("Sub Control marker ::", highlighter->subControlMarkerFormat());
  populateItem("Sub Control marker ::",
               highlighter->badSubControlMarkerFormat());
  populateItem("Pseudo State", highlighter->pseudoStateFormat());
  populateItem("Bad Pseudo State", highlighter->badPseudoStateFormat());
  populateItem("Pseudo State Marker :", highlighter->pseudoStateMarkerFormat());
  populateItem("Bad Pseudo State Marker :",
               highlighter->badPseudoStateMarkerFormat());
  populateItem("Property Name", highlighter->propertyFormat());
  populateItem("Bad Property Name", highlighter->badPropertyFormat());
  populateItem("Property Name Marker :", highlighter->propertyMarkerFormat());
  populateItem("Property Value", highlighter->propertyValueFormat());
  populateItem("Bad Property Value", highlighter->badPropertyValueFormat());
  populateItem("Property End Marker ;", highlighter->propertyEndMarkerFormat());
  populateItem("Start Brace {", highlighter->startBraceFormat());
  populateItem("Bad Start Brace {", highlighter->badStartBraceFormat());
  populateItem("End Brace }", highlighter->endBraceFormat());
  populateItem("Bad End Brace }", highlighter->badEndBraceFormat());
  populateItem("Mismatched Paranthesis {}", highlighter->braceMatchFormat());
  populateItem("Bad Mismatched Paranthesis {}",
               highlighter->badBraceMatchFormat());
  populateItem("/* Comment */", highlighter->commentFormat());
}

QTextCharFormat
TypeModel::format(int row)
{
  return m_formats.at(row);
}

QTextCharFormat
TypeModel::originalFormat(int row)
{
  return m_originalFormats.at(row);
}

void
TypeModel::setFormat(int row, QTextCharFormat format)
{
  QModelIndex i = index(row);
  m_formats.replace(row, format);
  emit dataChanged(i, i);
}

QList<QTextCharFormat>
TypeModel::formats() const
{
  return m_formats;
}

void
TypeModel::populateItem(const QString& text, QTextCharFormat format)
{
  m_fontfamily = format.fontFamily();
  m_text.append(text);
  m_formats.append(format);
  m_originalFormats.append(format);
}

ModifyFrame::ModifyFrame(QWidget* parent)
  : QWidget(parent)
{
  auto layout = new QVBoxLayout;
  setLayout(layout);

  auto colorLayout = new QFormLayout;
  m_foregroundBtn = new QPushButton(this);
  m_foregroundBtn->setAutoFillBackground(true);
  colorLayout->addRow(tr("Foreground"), m_foregroundBtn);
  connect(m_foregroundBtn,
          &QPushButton::clicked,
          this,
          &ModifyFrame::foregroundClicked);
  m_backgroundBtn = new QPushButton(this);
  m_backgroundBtn->setAutoFillBackground(true);
  colorLayout->addRow(tr("Background"), m_backgroundBtn);
  connect(m_backgroundBtn,
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
  m_boldBox = new QCheckBox(tr("Bold"), this);
  connect(m_boldBox, &QCheckBox::clicked, this, &ModifyFrame::boldClicked);
  fontLayout->addWidget(m_boldBox, 1, 0);
  m_italicBox = new QCheckBox(tr("Italic"), this);
  connect(m_italicBox, &QCheckBox::clicked, this, &ModifyFrame::italicClicked);
  fontLayout->addWidget(m_italicBox, 1, 1);
  layout->addLayout(fontLayout);

  auto underlineLayout = new QGridLayout;
  lbl = new QLabel("Underline");
  lbl->setFont(font);
  underlineLayout->addWidget(lbl, 0, 0);
  lbl = new QLabel("Color");
  underlineLayout->addWidget(lbl, 1, 0);
  m_underlineColorBtn = new QPushButton(this);
  m_underlineColorBtn->setAutoFillBackground(true);
  connect(m_underlineColorBtn,
          &QPushButton::clicked,
          this,
          &ModifyFrame::colorClicked);
  underlineLayout->addWidget(m_underlineColorBtn, 1, 1);
  QStringList values;
  values << "No Underline"
         << "Single Underline"
         << "Dash Underline"
         << "Dot Underline"
         << "Dash-Dot Underline"
         << "Dash-Dot-Dot Underline"
         << "Wave Underline";
  m_underlineType = new QComboBox(this);
  m_underlineType->addItems(values);
  connect(m_underlineType,
          qOverload<int>(&QComboBox::currentIndexChanged),
          this,
          &ModifyFrame::underlineTypeChanged);
  underlineLayout->addWidget(m_underlineType, 2, 0);
  layout->addLayout(underlineLayout);
}

void
ModifyFrame::setRow(int row, QTextCharFormat format)
{
  m_row = row;
  m_format = format;
  QPalette palette = m_foregroundBtn->palette();
  palette.setColor(m_foregroundBtn->backgroundRole(),
                   m_format.foreground().color());
  m_foregroundBtn->setPalette(palette);
  palette = m_backgroundBtn->palette();
  palette.setColor(m_backgroundBtn->backgroundRole(),
                   m_format.background().color());
  m_backgroundBtn->setPalette(palette);

  m_boldBox->setChecked(m_format.font().bold());
  m_italicBox->setChecked(m_format.font().italic());

  palette = m_underlineColorBtn->palette();
  palette.setColor(m_underlineColorBtn->backgroundRole(),
                   m_format.underlineColor());
  m_underlineColorBtn->setPalette(palette);

  m_underlineType->setCurrentIndex(m_format.underlineStyle());
}

void
ModifyFrame::foregroundClicked(bool)
{
  auto brush = m_format.foreground();
  auto dlg = new QColorDialog(brush.color(), this);
  if (dlg->exec() == QDialog::Accepted) {
    brush.setColor(dlg->currentColor());
    m_format.setForeground(brush);
    QPalette palette = m_foregroundBtn->palette();
    palette.setColor(m_foregroundBtn->backgroundRole(), dlg->currentColor());
    m_foregroundBtn->setPalette(palette);
    emit rowChanged(m_row, m_format);
  }
}

void
ModifyFrame::backgroundClicked(bool)
{
  auto brush = m_format.background();
  auto dlg = new QColorDialog(brush.color(), this);
  if (dlg->exec() == QDialog::Accepted) {
    brush.setColor(dlg->currentColor());
    m_format.setBackground(brush);
    QPalette palette = m_backgroundBtn->palette();
    palette.setColor(m_backgroundBtn->backgroundRole(), dlg->currentColor());
    m_backgroundBtn->setPalette(palette);
    emit rowChanged(m_row, m_format);
  }
}

void
ModifyFrame::colorClicked(bool)
{
  auto dlg = new QColorDialog(m_format.underlineColor(), this);
  if (dlg->exec() == QDialog::Accepted) {
    m_format.setUnderlineColor(dlg->currentColor());
    QPalette palette = m_underlineColorBtn->palette();
    palette.setColor(m_underlineColorBtn->backgroundRole(),
                     dlg->currentColor());
    m_underlineColorBtn->setPalette(palette);
    emit rowChanged(m_row, m_format);
  }
}

void
ModifyFrame::boldClicked(bool checked)
{
  auto font = m_format.font();
  font.setBold(checked);
  m_format.setFont(font);
  emit rowChanged(m_row, m_format);
}

void
ModifyFrame::italicClicked(bool checked)
{
  auto font = m_format.font();
  font.setItalic(checked);
  m_format.setFont(font);
  emit rowChanged(m_row, m_format);
}

void
ModifyFrame::underlineTypeChanged(int index)
{
  m_format.setUnderlineStyle(QTextCharFormat::UnderlineStyle(index));
  emit rowChanged(m_row, m_format);
}
