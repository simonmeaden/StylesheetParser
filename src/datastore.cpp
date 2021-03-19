/*
  Copyright 2020 Simon Meaden

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#include "datastore.h"
#include "node.h"
#include "parser.h"
//#include "safe_lib.h"
#include "qyamlcpp/qyamlcpp.h"
#include "rapidfuzz/fuzz.hpp"
#include "rapidfuzz/utils.hpp"
#include "string"
#include "stylesheetedit/stylesheetedit.h"
#include "stylesheetedit_p.h"

const QFont DataStore::NORMALFONT =
  QFont("Source Code Pro", 9, QFont::Normal, false);
const QFont DataStore::LIGHTFONT =
  QFont("Source Code Pro", 9, QFont::Light, false);
const QFont DataStore::BOLDFONT =
  QFont("Source Code Pro", 9, QFont::Bold, false);

DataStore::DataStore(QWidget* parent)
  : QObject(parent)
  , m_parent(parent)
  , m_invalidIcon(":/icons/invalid")
  , m_validIcon(":/icons/valid")
  , m_addSemiColonIcon(":/icons/add-scolon")
  , m_addDColonIcon(":/icons/add-dcolon")
  , m_addColonIcon(":/icons/add-colon")
  , m_badSColonIcon(":/icons/bad-scolon")
  , m_badColonIcon(":/icons/bad-colon")
  , m_badDColonIcon(":/icons/bad-dcolon")
  , m_noIcon(":/icons/no")
  , m_fuzzyIcon(":/icons/fuzzy")
  , m_braceCount(0)
  , m_manualMove(false)
  , m_hasSuggestion(false)
  , m_maxSuggestionCount(30)
{
  setConfigurationLocation();
  readQtCreatorTheme();
  readStandardThemes();
  readCustomThemes();
  loadConfig();

  m_currentTheme = m_qtTheme;
  m_currentThemeName = m_qtThemeName;

  initialiseWidgetModel();
}

DataStore::~DataStore()
{
  emit finished();
}

void
DataStore::readQtCreatorTheme()
{
  QDir dir(m_qtConfigDir);
  QSettings settings(QDir(m_qtConfigDir).filePath(m_qtConfigFile),
                     QSettings::IniFormat);
  settings.beginGroup("Core");
  const QStringList childKeys = settings.childKeys();
  for (auto childKey : childKeys) {
    if (childKey == "CreatorTheme") {
      m_qtTheme = settings.value(childKey).toString();
      if (m_qtTheme == "flat")
        m_qtThemeName = "Flat";
      else if (m_qtTheme == "flat-dark")
        m_qtThemeName = "Flat Dark";
      else if (m_qtTheme == "design-light")
        m_qtThemeName = "Design Light";
      else if (m_qtTheme == "design-dark")
        m_qtThemeName = "Design Dark";
      else if (m_qtTheme == "dark")
        m_qtThemeName = "Dark";
      else if (m_qtTheme == "default")
        m_qtThemeName = "Classic";
    }
  }
  settings.endGroup();
}

void
DataStore::setConfigurationLocation()
{
  m_qtConfigDir =
    QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
  m_qtConfigDir += "/QtProject";
  m_configDir = m_qtConfigDir + "/qtcreator/stylesheetedit";
  QDir dir;
  dir.mkpath(m_configDir);
  m_qtConfigFile = "QtCreator.ini";
  m_configFile = "stylesheetedit.yaml";
}

QString
DataStore::configDir() const
{
  return m_configDir;
}

QString
DataStore::configFile() const
{
  return m_configFile;
}

QString
DataStore::qtTheme() const
{
  return m_qtTheme;
}

QString
DataStore::qtThemeName() const
{
  return m_qtThemeName;
}

bool
DataStore::loadXmlTheme(const QString& /*name*/)
{
  return false;
}

bool
DataStore::saveXmlScheme(const QString& name)
{
  QString filename = QDir(m_configDir).filePath(name + ".xml");
  QFile file(filename);
  QMap<QString, QList<QString>> names;
  if (file.exists()) {
    // TODO OVERWRITE ??
    if (QMessageBox::question(
          m_parent, tr("File already exists!"), tr("Press Yes to Overwrite")) ==
        QMessageBox::No) {
      return false;
    }
  }

  if (file.open(QFile::ReadOnly | QFile::Text)) {
    QXmlStreamWriter writer(&file);
    writer.writeStartDocument();
    writer.writeStartElement(QStringLiteral("style-scheme"));
    writer.writeAttribute("version", "1.0");
    writer.writeAttribute("name", name);

    //          writer.writeAttribute(XbelReader::versionAttribute(),
    //          QStringLiteral("1.0")); for (int i = 0; i <
    //          treeWidget->topLevelItemCount(); ++i)
    //              writeItem(treeWidget->topLevelItem(i));

    writer.writeEndDocument();
    return true;
  }

  return false;
}

bool
DataStore::loadConfig(const QString& filename)
{
  QFile* file;
  if (filename.isEmpty())
    file = new QFile(QDir(m_configDir).filePath(m_configFile));
  else
    file = new QFile(filename);

  if (file->exists()) {
    auto config = YAML::LoadFile(*file);
    YAML::Node stylesheetedit, node, subnode1, subnode2;
    if (config["theme"]) {
      m_currentTheme = config["theme"].as<QString>();
    }
    if (config["themename"]) {
      m_currentThemeName = config["themename"].as<QString>();
    }
    return true;
  } else {
    saveConfig();
  }
  return false;
}

bool
DataStore::saveConfig(const QString& filename)
{
  QFile* file;
  if (filename.isEmpty())
    file = new QFile(QDir(m_configDir).filePath(m_configFile));
  else
    file = new QFile(filename);

  if (file->open((QFile::ReadWrite | QFile::Truncate))) {
    YAML::Emitter emitter;
    emitter << YAML::Key << "theme";
    emitter << YAML::Value << m_currentTheme;
    emitter << YAML::Key << "themename";
    emitter << YAML::Value << m_currentThemeName;

    QTextStream out(file);
    out << emitter.c_str();
    file->close();
    return true;
  }
  return false;
}

QString
DataStore::currentTheme() const
{
  return m_currentTheme;
}

QString
DataStore::currentThemeName() const
{
  return m_currentThemeName;
}

void
DataStore::readStyleFile(QFileInfo info)
{
  QFile file = info.absoluteFilePath();
  QMap<QString, QList<QString>> names;
  if (file.exists()) {
    if (file.open(QFile::ReadOnly | QFile::Text)) {
      QXmlStreamReader reader(&file);
      QStringRef attname;
      QString name, fore, back, underlineColor, underlineStyle, themename,
        italic, bold;
      bool text = false, linenumber = false, parenthesis = false,
           mismatch = false, currentlinenumber = false, punctuation = false,
           comment = false, ignore = false, currentline = false;
      Theme* theme = nullptr;
      while (reader.readNextStartElement()) {
        if (text && linenumber && parenthesis && mismatch &&
            currentlinenumber && currentline && comment && punctuation) {
          break;
        }

        auto attributes = reader.attributes();
        if (reader.name() == "style") {
          for (auto& att : attributes) {
            attname = att.name();
            if (attname == "name") {
              name = att.value().toString();
              if (name == "Text") {
                text = true;
              } else if (name == "LineNumber") {
                linenumber = true;
              } else if (name == "Parentheses") {
                parenthesis = true;
              } else if (name == "ParenthesesMismatch") {
                mismatch = true;
              } else if (name == "CurrentLineNumber") {
                currentlinenumber = true;
              } else if (name == "CurrentLine") {
                currentline = true;
              } else if (name == "Comment") {
                comment = true;
              } else if (name == "Comment") {
                punctuation = true;
              } else {
                ignore = true;
                reader.skipCurrentElement();
              }
            } else if (attname == "foreground") {
              fore = att.value().toString();
            } else if (attname == "background") {
              back = att.value().toString();
            } else if (attname == "italic") {
              italic = att.value().toString();
            } else if (attname == "bold") {
              bold = att.value().toString();
            } else if (attname == "underlineColor") {
              underlineColor = att.value().toString();
            } else if (attname == "underlineStyle") {
              underlineStyle = att.value().toString();
            }
          }
          if (!ignore) {
            if (theme) {
              ThemeData data(
                fore, back, italic, bold, underlineColor, underlineStyle);
              theme->addData(name, data);
              reader.skipCurrentElement();
            }
          }
          ignore = false;
        } else if (reader.name() == "style-scheme") {
          for (auto& att : attributes) {
            if (att.name() == "name") {
              theme = new Theme();
              themename = att.value().toString();
            }
          }
        }
      }
      m_customThemes.insert(themename, theme);
      theme = nullptr;
    }
  }
}

void
DataStore::readStandardThemes()
{
  auto qtdir = getenv("QTDIR");
  QDir dir(qtdir);
  dir.cdUp();
  dir.cdUp();
  auto configPath = dir.absolutePath();
  configPath += "/Tools/QtCreator/share/qtcreator/styles";
  dir.setPath(configPath);
  if (dir.exists()) {
    QStringList filters;
    filters << "*.xml";
    dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    dir.setNameFilters(filters);
    auto files = dir.entryInfoList(filters);
    for (auto& info : files) {
      readStyleFile(info);
    }
  }
}

void
DataStore::readCustomThemes()
{
  auto configPath =
    QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
  configPath += "/QtProject/qtcreator/styles";
  QDir dir = (configPath);
  if (dir.exists()) {
    QStringList filters;
    filters << "*.xml";
    dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    dir.setNameFilters(filters);
    auto files = dir.entryInfoList(filters);
    for (auto& info : files) {
      readStyleFile(info);
    }
  }
}

void
DataStore::addWidget(const QString& widget, const QString& parent)
{
  QMutexLocker locker(&m_mutex);
  m_widgetModel->addWidget(widget, parent);
}

void
DataStore::removeWidget(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  m_widgetModel->removeWidget(name);
}

bool
DataStore::containsWidget(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  // NOT toLower() as widget names are cased.
  return m_widgetModel->hasWidget(name);
}

QMultiMap<int, QString>
DataStore::fuzzySearchWidgets(const QString& name)
{
  return m_widgetModel->fuzzySearchWidgets(name);
}

bool
DataStore::containsProperty(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return m_widgetModel->containsProperty(name.toLower());
}

QMultiMap<int, QString>
DataStore::fuzzySearchProperty(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return m_widgetModel->fuzzySearchProperty(name);
}

QMultiMap<int, QString>
DataStore::fuzzySearchPropertyValue(const QString& name, const QString& value)
{
  return m_widgetModel->fuzzySearchPropertyValue(name, value);
}

bool
DataStore::containsStylesheetProperty(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return m_StylesheetProperties.contains(name.toLower());
}

bool
DataStore::containsPseudoState(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return m_widgetModel->containsPseudoState(name.toLower());
}

QMultiMap<int, QString>
DataStore::fuzzySearchPseudoStates(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return m_widgetModel->fuzzySearchPseudoStates(name);
}

bool
DataStore::containsSubControl(const QString& name)
{
  return m_widgetModel->containsSubControl(name);
}

bool
DataStore::isValidSubControlForWidget(const QString& widget,
                                      const QString& subcontrol)
{
  return m_widgetModel->isValidSubControlForWidget(widget, subcontrol);
}

QMultiMap<int, QString>
DataStore::fuzzySearchSubControl(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return m_widgetModel->fuzzySearchSubControl(name);
}

bool
DataStore::addCustomWidget(const QString& name, const QString& parent)
{
  return m_widgetModel->addCustomWidget(name, parent);
}

bool
DataStore::addCustomWidgetPseudoStates(const QString& name,
                                       const QStringList& states)
{
  return m_widgetModel->addCustomWidgetPseudoStates(name, states);
}

bool
DataStore::addCustomWidgetSubControls(const QString& name,
                                      const QStringList& controls)
{
  return m_widgetModel->addCustomWidgetSubControls(name, controls);
}

bool
DataStore::addCustomWidgetProperties(const QString& name,
                                     const QStringList& properties)
{
  return m_widgetModel->addCustomWidgetProperties(name, properties);
}

bool
DataStore::addCustomWidgetPropertyValue(const QString& widget,
                                        const QString& property,
                                        const QString& value)
{
  return m_widgetModel->addCustomWidgetPropertyValue(widget, property, value);
}

bool
DataStore::addCustomWidgetPropertyValues(const QString& widget,
                                         const QString& property,
                                         QStringList values)
{
  return m_widgetModel->addCustomWidgetPropertyValues(widget, property, values);
}

QList<QString>
DataStore::customThemeNames() const
{
  return m_customThemes.keys();
}

QMap<QString, Theme*>
DataStore::customThemes() const
{
  return m_customThemes;
}

bool
DataStore::checkSubControlForWidget(const QString& widget, const QString& name)
{
  return m_widgetModel->checkSubControlForWidget(widget, name);
}

PropertyStatus*
WidgetModel::checkAlignment(const QString& value) const
{
  auto status = new PropertyStatus();
  if (m_alignmentValues.contains(value)) {
    status->state = PropertyStatus::GoodPropertyValue;
  } else {
    status->state = PropertyStatus::IrrelevantValue;
  }
  return status;
}

PropertyStatus*
WidgetModel::checkAttachment(const QString& value) const
{
  auto status = new PropertyStatus();
  if (m_attachment.contains(value)) {
    status->state = PropertyStatus::GoodPropertyValue;
  } else {
    status->state = PropertyStatus::IrrelevantValue;
  }
  return status;
}

PropertyStatus*
WidgetModel::checkBackground(const QString& value) const
{
  auto status = checkBrush(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status = checkUrl(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status = checkRepeat(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status = checkAlignment(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status->state = PropertyStatus::IrrelevantValue;
  return status;
}

PropertyStatus*
WidgetModel::checkBool(const QString& value) const
{
  auto status = new PropertyStatus();
  if (value == "true" || value == "false") {
    status->state = PropertyStatus::GoodPropertyValue;
  } else {
    status->state = PropertyStatus::IrrelevantValue;
  }
  return status;
}

PropertyStatus*
WidgetModel::checkBoolean(const QString& value) const
{
  auto status = new PropertyStatus();
  if (value == "0" || value == "1") {
    status->state = PropertyStatus::GoodPropertyValue;
  } else {
    status->state = PropertyStatus::IrrelevantValue;
  }
  return status;
}

PropertyStatus*
WidgetModel::checkBorder(const QString& value) const
{
  auto status = checkBorderStyle(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status = checkLength(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status = checkBrush(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status->state = PropertyStatus::IrrelevantValue;
  return status;
}

PropertyStatus*
WidgetModel::checkBorderImage(const QString& value) const
{
  auto status = checkUrl(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status = checkNumber(value);
  if (status->notIrrelevant()) {
    return status;
  }

  if (m_borderImage.contains(value)) {
    status->state = PropertyStatus::GoodPropertyValue;
  } else {
    status->state = PropertyStatus::IrrelevantValue;
  }
  return status;
}

PropertyStatus*
WidgetModel::checkBorderStyle(const QString& value) const
{
  auto status = new PropertyStatus();
  if (m_borderStyle.contains(value)) {
    status->state = PropertyStatus::GoodPropertyValue;
  } else {
    status->state = PropertyStatus::IrrelevantValue;
  }
  return status;
}

PropertyStatus*
WidgetModel::checkBoxColors(const QString& value) const
{
  return checkColor(value);
}

PropertyStatus*
WidgetModel::checkBoxLengths(const QString& value) const
{
  return checkLength(value);
}

PropertyStatus*
WidgetModel::checkBrush(const QString& value) const
{
  auto status = checkColor(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status = checkGradient(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status = checkPaletteRole(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status->state = PropertyStatus::IrrelevantValue;
  return status;
}

PropertyStatus*
WidgetModel::checkColor(const QString& value) const
{
  auto status = new PropertyStatus();
  // check if this was a color name.
  if (m_colors.contains(value)) {
    status->state = PropertyStatus::GoodPropertyValue;
    return status;
  }

  if (value.startsWith("#")) {
    if (value.length() != 4 || value.length() != 7) {
      status->state = PropertyStatus::BadPropertyValue;
    } else {
      bool ok = false;
      QString val = value.right(value.length() - 1);
      val.toUInt(&ok, 16);
      if (ok) {
        status->state = PropertyStatus::GoodPropertyValue;
      }
    }
    return status;
  }

  if (value.startsWith("rgba")) {
    QString rgb = value.mid(value.indexOf('('), value.indexOf(')'));
    QStringList cols = rgb.split(',');

    if (cols.count() != 4) {
      status->state = PropertyStatus::BadPropertyValue;
    } else {
      bool ok = false;
      int val, count = 0;
      for (int i = 0; i < 4; i++) {
        auto col = cols.at(i).trimmed();

        if (col.endsWith('%')) {
          col = col.left(col.length() - 1);
          val = col.toInt(&ok);

          if (val >= 0 && val <= 100) {
            count++;
          }

        } else {
          val = col.toInt(&ok);

          if (val >= 0 && val <= 255) {
            count++;
          }
        }
      }

      if (count == 4) {
        status->state = PropertyStatus::GoodPropertyValue;
      } else {
        status->state = PropertyStatus::BadPropertyValue;
      }
    }
    return status;
  }

  if (value.startsWith("rgb")) { // include rgba in this case
    QString rgb = value.mid(value.indexOf('('), value.indexOf(')'));
    QStringList cols = rgb.split(',');

    if (cols.count() != 3) {
      status->state = PropertyStatus::BadPropertyValue;
      return status;
    } else {
      bool ok = false;
      int val, count = 0;
      for (int i = 0; i < 3; i++) {
        auto col = cols.at(i).trimmed();

        if (col.endsWith('%')) {
          col = col.left(col.length() - 1);
          val = col.toInt(&ok);

          if (val >= 0 && val <= 100) {
            count++;
          }

        } else {
          val = col.toInt(&ok);

          if (val >= 0 && val <= 255) {
            count++;
          }
        }
      }

      if (count == 3) {
        status->state = PropertyStatus::GoodPropertyValue;
      } else {
        status->state = PropertyStatus::BadPropertyValue;
      }
    }
    return status;
  }

  if (value.startsWith("hsla") || value.startsWith("hsva")) {
    QString rgb = value.mid(value.indexOf('('), value.indexOf(')'));
    QStringList cols = rgb.split(',');

    if (cols.count() != 4) {
      status->state = PropertyStatus::BadPropertyValue;
      return status;
    }

    bool ok = false;
    int val, count = 0;

    for (int i = 0; i < 4; i++) {
      auto col = cols.at(i).trimmed();

      if (col.endsWith('%')) {
        col = col.left(col.length() - 1);
        val = col.toInt(&ok);

        if (i == 0) {
          // hue cannot be a percentage.
          // a percentage here will cause a fail.
          continue;

        } else if (val >= 0 && val <= 100) {
          count++;
        }

      } else {
        val = col.toInt(&ok);

        if (i == 0 && val >= 0 && val < 360) {
          count++;

        } else if (val >= 0 && val <= 100) {
          count++;
        }
      }
    }

    if (count == 4) {
      status->state = PropertyStatus::GoodPropertyValue;
    } else {
      status->state = PropertyStatus::BadPropertyValue;
    }
    return status;
  }

  if (value.startsWith("hsl") || value.startsWith("hsv")) {
    QString rgb = value.mid(value.indexOf('('), value.indexOf(')'));
    QStringList cols = rgb.split(',');

    if (cols.count() != 3) {
      status->state = PropertyStatus::BadPropertyValue;
      return status;
    } else {

      bool ok = false;
      int val, count = 0;

      for (int i = 0; i < 3; i++) {
        auto col = cols.at(i).trimmed();

        if (col.endsWith('%')) {
          col = col.left(col.length() - 1);
          val = col.toInt(&ok);

          if (i == 0) {
            // hue cannot be a percentage.
            // a percentage here will cause a fail.
            continue;

          } else if (val >= 0 && val <= 100) {
            count++;
          }

        } else {
          val = col.toInt(&ok);

          if (i == 0 && val >= 0 && val < 360) {
            count++;

          } else if (val >= 0 && val <= 100) {
            count++;
          }
        }
      }

      if (count == 3) {
        PropertyStatus::GoodPropertyValue;
      } else {
        status->state = PropertyStatus::BadPropertyValue;
      }
    }
    return status;
  }

  auto fuzzylist = fuzzySearch(value, m_colors);
  if (!fuzzylist.isEmpty()) {
    status->state = PropertyStatus::FuzzyColorValue;
    return status;
  }

  status->state = PropertyStatus::IrrelevantValue;
  return status;
}

PropertyStatus*
WidgetModel::checkFontStyle(const QString& value) const
{
  auto status = new PropertyStatus();
  if (m_fontStyle.contains(value)) {
    status->state = PropertyStatus::GoodPropertyValue;
  } else {
    status->state = PropertyStatus::IrrelevantValue;
  }
  return status;
}

PropertyStatus*
WidgetModel::checkFont(const QString& value) const
{
  auto status = checkFontSize(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status = checkFontStyle(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status = checkFontWeight(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status->state = PropertyStatus::IrrelevantValue;
  return status;
}

PropertyStatus*
WidgetModel::checkFontSize(const QString& value) const
{
  auto status = checkLength(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status->state = PropertyStatus::IrrelevantValue;
  return status;
}

PropertyStatus*
WidgetModel::checkFontWeight(const QString& value) const
{
  auto status = new PropertyStatus();
  if (m_fontWeight.contains(value)) {
    status->state = PropertyStatus::GoodPropertyValue;
    return status;
  }

  bool ok;
  value.toInt(&ok);
  if (ok) {
    status->state = PropertyStatus::GoodPropertyValue;
    return status;
  }

  status->state = PropertyStatus::IrrelevantValue;
  return status;
}

PropertyStatus*
WidgetModel::checkGradient(const QString& value) const
{
  auto status = new PropertyStatus();
  QString cleanValue = value.simplified().toLower();
  bool contains = false;

  for (auto& gradient : m_gradient) {
    if (cleanValue.contains(gradient)) {
      contains = true;
      break;
    }
    auto left = cleanValue.split("(").first();
    double score =
      rapidfuzz::fuzz::ratio(gradient.toStdString(), left.toStdString());
    if (score < 100.0 && score > 90.0) { // TODO Increase minimum ??
      status->state = PropertyStatus::FuzzyGradientName;
      return status;
    }
  }

  if (!contains) {
    status->state = PropertyStatus::IrrelevantValue;
    return status;
  }

  auto sections =
    cleanValue.split(QRegularExpression("[,()]"), Qt::SkipEmptyParts);

  bool linear = false;
  bool radial = false;
  bool conical = false;
  QString name, number, color;

  for (auto& section : sections) {
    if (!(linear || radial || conical)) {
      if (section == "qlineargradient") {
        linear = true;
        continue;
      } else if (section == "qradialgradient") {
        radial = true;
        continue;
      } else if (section == "qconicalgradient") {
        conical = true;
        continue;
      } else {
        status->state = PropertyStatus::BadGradientName; // bad name
        return status;
      }
    }
    auto parts =
      section.split(QRegularExpression("[\\s:]"), Qt::SkipEmptyParts);

    // only two or three parts is valid.
    if (parts.size() < 2 || parts.size() > 3) {
      status->state = PropertyStatus::BadGradientValue;
      return status;
    }

    name = parts.at(0);
    number = parts.at(1);
    if (parts.size() == 3) {
      color = parts.at(2);
    }

    if (linear) {
      if (name == "x1" || name == "y1" || name == "x2" || name == "y2") {
        if (parts.size() == 3) {
          status->state = PropertyStatus::BadGradientValueCount;
        } else if (!checkGradientNumber(number.trimmed()))
          status->state = PropertyStatus::BadGradientValue;
        return status;
      } else if (name == "stop") {
        if (parts.size() == 2)
          status->state = PropertyStatus::BadGradientValueCount;
        if (!checkGradientNumber(number.trimmed()))
          status->state = PropertyStatus::BadGradientNumericalValue;
        if (!checkGradientColor(color.trimmed()))
          status->state = PropertyStatus::BadGradientColorValue;
        return status;
      } else {
        // an invalid name.
        status->state = PropertyStatus::BadGradientValueName;
        return status;
      }
    } else if (radial) {
      if (name == "cx" || name == "cy" || name == "angle") {
        if (parts.size() == 3) {
          status->state = PropertyStatus::BadGradientValueCount;
        } else if (!checkGradientNumber(number.trimmed()))
          status->state = PropertyStatus::BadGradientValue;
        return status;
      } else if (name == "stop") {
        if (parts.size() == 2)
          status->state = PropertyStatus::BadGradientValueCount;
        if (!checkGradientNumber(number.trimmed()))
          status->state = PropertyStatus::BadGradientNumericalValue;
        if (!checkGradientColor(color.trimmed()))
          status->state = PropertyStatus::BadGradientColorValue;
        return status;
      } else {
        // an invalid name.
        status->state = PropertyStatus::BadGradientValueName;
        return status;
      }
    } else if (conical) {
      if (name == "cx" || name == "cy" || name == "radius" || name == "fx" ||
          name == "fy") {
        if (parts.size() == 3) {
          status->state = PropertyStatus::BadGradientValueCount;
        } else if (!checkGradientNumber(number.trimmed()))
          status->state = PropertyStatus::BadGradientValue;
        return status;
      } else if (name == "stop") {
        if (parts.size() == 2)
          status->state = PropertyStatus::BadGradientValueCount;
        if (!checkGradientNumber(number.trimmed()))
          status->state = PropertyStatus::BadGradientNumericalValue;
        if (!checkGradientColor(color.trimmed()))
          status->state = PropertyStatus::BadGradientColorValue;
        return status;
      } else {
        // an invalid name.
        status->state = PropertyStatus::BadGradientValueName;
        return status;
      }
    } /* else {
       return BadGradientName;
     }*/
  }

  status->state = PropertyStatus::IrrelevantValue;
  return status;
}

bool
WidgetModel::checkGradientColor(const QString& value) const
{
  QColor color(value);
  if (!color.isValid()) {
    return false;
  }
  return true;
}

bool
WidgetModel::checkGradientNumber(const QString& value) const
{
  bool ok;
  value.toDouble(&ok);
  if (!ok) {
    return false;
  }
  return true;
}

PropertyStatus*
WidgetModel::checkIcon(const QString& value) const
{
  auto status = checkUrl(value);
  if (status->notIrrelevant()) {
    return status;
  }

  if (m_icon.contains(value)) {
    status->state = PropertyStatus::GoodPropertyValue;
    return status;
  }

  status->state = PropertyStatus::IrrelevantValue;
  return status;
}

PropertyStatus*
WidgetModel::checkLength(const QString& value) const
{
  auto status = new PropertyStatus();
  bool ok;

  if (value.endsWith("px") || value.endsWith("pt") || value.endsWith("em") ||
      value.endsWith("ex")) {
    auto numValue = value.left(value.length() - 2);
    numValue.toDouble(&ok);
    if (ok) {
      status->state = PropertyStatus::GoodPropertyValue;
    } else {
      status->state = PropertyStatus::BadPropertyValue;
    }
    return status;
  } else if (value.at(value.length() - 1).isDigit()) {
    auto numValue = value.left(value.length() - 2);
    numValue.toUInt(&ok);
    if (ok) {
      status->state = PropertyStatus::GoodPropertyValue;
    } else {
      status->state = PropertyStatus::BadPropertyValue;
    }
    return status;
  }

  status->state = PropertyStatus::IrrelevantValue;
  return status;
}

PropertyStatus*
WidgetModel::checkNumber(const QString& value) const
{
  auto status = new PropertyStatus();
  bool ok;
  // don't actually need the value, I only want to know
  // if it actually is a number.
  value.toDouble(&ok);
  if (ok) {
    status->state = PropertyStatus::GoodPropertyValue;
  }

  status->state = PropertyStatus::IrrelevantValue;
  return status;
}

PropertyStatus*
WidgetModel::checkOutline(const QString& value) const
{
  auto status = checkOutlineColor(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status = checkOutlineColor(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status = checkOutlineStyle(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status = checkOutlineWidth(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status = checkOutlineOffset(value);
  if (status->notIrrelevant()) {
    return status;
  }

  status->state = PropertyStatus::IrrelevantValue;
  return status;
}

PropertyStatus*
WidgetModel::checkOrigin(const QString& value) const
{
  auto status = new PropertyStatus();
  if (m_origin.contains(value)) {
    status->state = PropertyStatus::GoodPropertyValue;
  } else {
    status->state = PropertyStatus::IrrelevantValue;
  }
  return status;
}

PropertyStatus*
WidgetModel::checkOutlineStyle(const QString& value) const
{
  auto status = new PropertyStatus();
  if (m_outlineStyle.contains(value)) {
    status->state = PropertyStatus::GoodPropertyValue;
  } else {
    status->state = PropertyStatus::IrrelevantValue;
  }
  return status;
}

PropertyStatus*
WidgetModel::checkOutlineColor(const QString& value) const
{
  if (value == m_outlineColor) {
    auto status = new PropertyStatus();
    status->state = PropertyStatus::GoodPropertyValue;
    return status;
  }
  return checkColor(value);
}

PropertyStatus*
WidgetModel::checkOutlineWidth(const QString& value) const
{
  if (m_outlineWidth.contains(value)) {
    auto status = new PropertyStatus();
    status->state = PropertyStatus::GoodPropertyValue;
    return status;
  }
  return checkLength(value);
}

PropertyStatus*
WidgetModel::checkOutlineOffset(const QString& value) const
{
  return checkLength(value);
}

PropertyStatus*
WidgetModel::checkOutlineRadius(const QString& value) const
{
  // check for 4 radius values.
  return checkRadius(value);
}

PropertyStatus*
WidgetModel::checkPaletteRole(const QString& value) const
{
  auto status = new PropertyStatus();
  if (m_paletteRoles.contains(value)) {
    status->state = PropertyStatus::GoodPropertyValue;
    return status;
  }

  status->state = PropertyStatus::IrrelevantValue;
  return status;
}

PropertyStatus*
WidgetModel::checkRadius(const QString& value) const
{
  return checkLength(value);
}

PropertyStatus*
WidgetModel::checkRepeat(const QString& value) const
{
  auto status = new PropertyStatus();
  if (m_repeat.contains(value)) {
    status->state = PropertyStatus::GoodPropertyValue;
    return status;
  }

  status->state = PropertyStatus::IrrelevantValue;
  return status;
}

PropertyStatus*
WidgetModel::checkUrl(const QString& value) const
{
  auto status = new PropertyStatus();
  auto cols = value.split("()");
  if (cols.size() == 2) {
    if (cols.at(0) == "url") {
      QUrl url(cols.at(1));
      if (url.isValid()) {
        status->state = PropertyStatus::GoodPropertyValue;
      } else {
        status->state = PropertyStatus::BadPropertyValue;
      }
      return status;
    }
  }

  status->state = PropertyStatus::IrrelevantValue;
  return status;
}

PropertyStatus*
WidgetModel::checkPosition(const QString& value) const
{
  auto status = new PropertyStatus();
  if (m_position.contains(value)) {
    status->state = PropertyStatus::GoodPropertyValue;
    return status;
  }

  status->state = PropertyStatus::IrrelevantValue;
  return status;
}

PropertyStatus*
WidgetModel::checkTextDecoration(const QString& value) const
{
  auto status = new PropertyStatus();
  if (m_textDecoration.contains(value)) {
    status->state = PropertyStatus::GoodPropertyValue;
    return status;
  }

  status->state = PropertyStatus::IrrelevantValue;
  return status;
}

void
DataStore::initialiseWidgetModel()
{
  m_widgetModel = new WidgetModel();
}

PropertyStatus*
WidgetModel::checkPropertyValue(AttributeType propertyAttribute,
                                const QString& valuename)
{
  PropertyStatus* status = nullptr;
  switch (propertyAttribute) {
    case Alignment:
      status = checkAlignment(valuename);
      break;

    case Attachment:
      status = checkAttachment(valuename);
      break;

    case Background:
      status = checkBackground(valuename);
      break;

    case Bool:
      status = checkBool(valuename);
      break;

    case Boolean:
      status = checkBoolean(valuename);
      break;

    case Border:
      status = checkBorder(valuename);
      break;

    case BorderImage:
      status = checkBorderImage(valuename);
      break;

    case BorderStyle:
      status = checkBorderStyle(valuename);
      break;

    case BoxColors:
      status = checkBoxColors(valuename);
      break;

    case BoxLengths:
      status = checkBoxLengths(valuename);
      break;

    case Brush:
      status = checkBrush(valuename);
      break;

    case Color:
      status = checkColor(valuename);
      break;

    case Font:
      status = checkFont(valuename);
      break;

    case FontSize:
      status = checkFontSize(valuename);
      break;

    case FontStyle:
      status = checkFontStyle(valuename);
      break;

    case FontWeight:
      status = checkFontWeight(valuename);
      break;

    case Gradient:
      status = checkGradient(valuename);
      break;

    case Icon:
      status = checkIcon(valuename);
      break;

    case Length:
      status = checkLength(valuename);
      break;

    case Number: // TODO not supported.
      status = checkNumber(valuename);
      break;

    case Origin:
      status = checkOrigin(valuename);
      break;

    case Outline:
      status = checkOutline(valuename);
      break;

    case OutlineRadius:
      status = checkOutlineRadius(valuename);
      break;

    case OutlineStyle:
      status = checkOutlineStyle(valuename);
      break;

    case PaletteRole:
      status = checkPaletteRole(valuename);
      break;

    case Position:
      status = checkPosition(valuename);
      break;

    case Radius:
      status = checkRadius(valuename);
      break;

    case Repeat:
      status = checkRepeat(valuename);
      break;

    case TextDecoration:
      status = checkTextDecoration(valuename);
      break;

    case Url:
      status = checkUrl(valuename);
      break;

    case String:
      // value IS a string.
      // might need to check the string contents here.
      status->state = PropertyStatus::GoodPropertyValue;
      break;

    case List:
      // This should never reach here.
      status->state = PropertyStatus::GoodPropertyValue;
      break;
  }

  if (!status) {
    status = new PropertyStatus(PropertyStatus::IrrelevantValue);
  }

  return status;
}

PropertyStatus*
DataStore::isValidPropertyValueForProperty(const QString& propertyname,
                                           const QString& valuename)
{
  return m_widgetModel->isValidPropertyValueForProperty(propertyname,
                                                        valuename);
}

QStringList
DataStore::possibleWidgetsForSubControl(const QString& name)
{
  return m_widgetModel->possibleWidgetsForSubControl(name);
}

QStringList
DataStore::possibleSubControlsForWidget(const QString& widget)
{
  return m_widgetModel->possibleSubControlsForWidget(widget);
}

void
DataStore::addSubControl(const QString& control, const QString& widget)
{
  QMutexLocker locker(&m_mutex);
  QStringList widgets;
  widgets << widget;
  m_widgetModel->addSubControl(control, widgets);
}

void
DataStore::addSubControl(const QString& control, QStringList& widgets)
{
  QMutexLocker locker(&m_mutex);
  m_widgetModel->addSubControl(control, widgets);
}

void
DataStore::removeSubControl(const QString& control)
{
  QMutexLocker locker(&m_mutex);
  m_widgetModel->removeSubControl(control.trimmed());
}

void
DataStore::addPseudoState(const QString& state)
{
  QMutexLocker locker(&m_mutex);
  m_widgetModel->addPseudoState(state, true);
}

void
DataStore::removePseudoState(const QString& state)
{
  QMutexLocker locker(&m_mutex);
  m_widgetModel->removePseudoState(state);
}

int
DataStore::braceCount()
{
  QMutexLocker locker(&m_mutex);
  return m_braceCount;
}

void
DataStore::setBraceCount(int value)
{
  QMutexLocker locker(&m_mutex);
  m_braceCount = value;
}

void
DataStore::incrementBraceCount()
{
  QMutexLocker locker(&m_mutex);
  m_braceCount++;
}

bool
DataStore::decrementBraceCount()
{
  QMutexLocker locker(&m_mutex);
  if (m_braceCount > 0) {
    m_braceCount--;
    return true;
  }
  return false;
}

// bool
// DataStore::isBraceCountZero()
//{
//  return m_braceStack.isEmpty();
//}

// void
// DataStore::pushStartBrace(StartBraceNode* startbrace)
//{
//  m_startbraces.append(startbrace);
//  m_braceStack.push(startbrace);
//}

// void
// DataStore::pushEndBrace(EndBraceNode* endbrace)
//{
//  m_endbraces.append(endbrace);
//  if (!isBraceCountZero()) {
//    auto startbrace = m_braceStack.pop();
//    endbrace->setStartNode(startbrace);
//    startbrace->setEndBrace(endbrace);
//  }
//}

QMap<QTextCursor, Node*>
DataStore::nodes()
{
  QMutexLocker locker(&m_mutex);
  return m_nodes;
}

void
DataStore::insertNode(QTextCursor cursor, Node* node)
{
  QMutexLocker locker(&m_mutex);
  m_nodes.insert(cursor, node);
  //  switch (node->type()) {
  //    case StartBraceType:
  //      pushStartBrace(qobject_cast<StartBraceNode*>(node));
  //      break;
  //    case EndBraceType:
  //      pushEndBrace(qobject_cast<EndBraceNode*>(node));
  //  }
}

bool
DataStore::isNodesEmpty()
{
  QMutexLocker locker(&m_mutex);
  return m_nodes.isEmpty();
}

void
DataStore::clearNodes()
{
  QMutexLocker locker(&m_mutex);
  m_nodes.clear();
}

void
DataStore::setNodes(QMap<QTextCursor, Node*> nodes)
{
  QMutexLocker locker(&m_mutex);
  m_nodes = nodes;
}

int
DataStore::maxSuggestionCount()
{
  QMutexLocker locker(&m_mutex);
  return m_maxSuggestionCount;
}

void
DataStore::setMaxSuggestionCount(int maxSuggestionCount)
{
  QMutexLocker locker(&m_mutex);
  m_maxSuggestionCount = maxSuggestionCount;
}

bool
DataStore::hasSuggestion()
{
  QMutexLocker locker(&m_mutex);
  return m_hasSuggestion;
}

void
DataStore::setHasSuggestion(bool suggestion)
{
  QMutexLocker locker(&m_mutex);
  m_hasSuggestion = suggestion;
}

bool
DataStore::isManualMove()
{
  QMutexLocker locker(&m_mutex);
  return m_manualMove;
}

void
DataStore::setManualMove(bool manualMove)
{
  QMutexLocker locker(&m_mutex);
  m_manualMove = manualMove;
}

Node*
DataStore::currentNode()
{
  QMutexLocker locker(&m_mutex);
  return m_currentNode;
}

void
DataStore::setCurrentWidget(WidgetNode* widget)
{
  QMutexLocker locker(&m_mutex);
  m_currentNode = widget;
}

bool
DataStore::isCurrentWidget(WidgetNode* node)
{
  QMutexLocker locker(&m_mutex);
  return (m_currentNode == node);
}

QTextCursor
DataStore::currentCursor()
{
  QMutexLocker locker(&m_mutex);
  return m_currentCursor;
}

void
DataStore::setCurrentCursor(const QTextCursor& currentCursor)
{
  QMutexLocker locker(&m_mutex);
  m_currentCursor = currentCursor;
}

QIcon
DataStore::invalidIcon() const
{
  return m_invalidIcon;
}

QIcon
DataStore::validIcon() const
{
  return m_validIcon;
}

QIcon
DataStore::addDColonIcon() const
{
  return m_addDColonIcon;
}

QIcon
DataStore::addSemiColonIcon() const
{
  return m_addSemiColonIcon;
}

QIcon
DataStore::addColonIcon() const
{
  return m_addColonIcon;
}

QIcon
DataStore::badSemiColonIcon() const
{
  return m_badSColonIcon;
}

QIcon
DataStore::badColonIcon() const
{
  return m_badColonIcon;
}

QIcon
DataStore::badDColonIcon() const
{
  return m_badDColonIcon;
}

QIcon
DataStore::noIcon() const
{
  return m_noIcon;
}

QIcon
DataStore::fuzzyIcon() const
{
  return m_fuzzyIcon;
}

AttributeType
DataStore::propertyValueAttribute(const QString& value)
{
  QMutexLocker locker(&m_mutex);
  return m_widgetModel->propertyValueAttribute(value);
}

// QMap<QString, AttributeType>
// DataStore::initialiseStylesheetMap()
//{
//  QMap<QString, AttributeType> map;
//  map.insert("widget", StylesheetEditGood);
//  map.insert("subcontrol", StylesheetEditGood);
//  map.insert("pseudostate", StylesheetEditGood);
//  map.insert("subcontrolmarker", StylesheetEditGood);
//  map.insert("pseudostatemarker", StylesheetEditGood);
//  map.insert("property", StylesheetEditGood);
//  map.insert("propertymarker", StylesheetEditGood);
//  map.insert("value", StylesheetEditGood);
//  map.insert("startbrace", StylesheetEditGood);
//  map.insert("endbrace", StylesheetEditGood);
//  map.insert("bracematch", StylesheetEditGood);
//  map.insert("comment", StylesheetEditGood);
//  map.insert("bad", StylesheetEditBad);
//  // TODO more values
//  return map;
//}

WidgetItem::WidgetItem(const QString& name, WidgetItem* parent)
  : m_name(name)
  , m_parent(parent)
{}

QString
WidgetItem::name()
{
  return m_name;
}

WidgetItem*
WidgetItem::parent()
{
  return m_parent;
}

QList<WidgetItem*>
WidgetItem::children()
{
  return m_children;
}

void
WidgetItem::addChild(WidgetItem* child)
{
  m_children.append(child);
}

void
WidgetItem::removeChild(const QString& name)
{
  for (auto child : m_children) {
    if (child->name() == name) {
      m_children.removeOne(child);
      break;
    }
  }
}

bool
WidgetItem::hasChildren()
{
  return m_children.isEmpty();
}

bool
WidgetItem::isExtraWidget() const
{
  return m_extraWidget;
}

void
WidgetItem::setExtraWidget(bool extraWidget)
{
  m_extraWidget = extraWidget;
}

void
WidgetItem::addSubcontrol(const QString& control)
{
  if (!m_subcontrols.contains(control))
    m_subcontrols.append(control);
}

bool
WidgetItem::removeSubcontrol(const QString& control)
{
  return m_subcontrols.removeOne(control);
}

bool
WidgetItem::hasSubControl(const QString& name)
{
  return m_subcontrols.contains(name);
}

QStringList
WidgetItem::subControls()
{
  return m_subcontrols;
}

void
WidgetModel::initPaletteRoles()
{
  m_paletteRoles << "alternate-base"
                 << "base"
                 << "bright-text"
                 << "button"
                 << "button-text"
                 << "dark"
                 << "highlight"
                 << "highlighted-text"
                 << "light"
                 << "link"
                 << "link-visited"
                 << "mid"
                 << "midlight"
                 << "shadow"
                 << "text"
                 << "window"
                 << "window-text";
}

void
WidgetModel::initColorNames()
{
  m_colors << "black"
           << "silver"
           << "gray"
           << "whitesmoke"
           << "maroon"
           << "red"
           << "purple"
           << "fuchsia"
           << "green"
           << "lime"
           << "olivedrab"
           << "yellow"
           << "navy"
           << "blue"
           << "teal"
           << "aquamarine"
           << "orange"
           << "aliceblue"
           << "antiquewhite"
           << "aqua"
           << "azure"
           << "beige"
           << "bisque"
           << "blanchedalmond"
           << "blueviolet"
           << "brown"
           << "burlywood"
           << "cadetblue"
           << "chartreuse"
           << "chocolate"
           << "coral"
           << "cornflowerblue"
           << "cornsilk"
           << "crimson"
           << "darkblue"
           << "darkcyan"
           << "darkgoldenrod"
           << "darkgray"
           << "darkgreen"
           << "darkgrey"
           << "darkkhaki"
           << "darkmagenta"
           << "darkolivegreen"
           << "darkorange"
           << "darkorchid"
           << "darkred"
           << "darksalmon"
           << "darkseagreen"
           << "darkslateblue"
           << "darkslategray"
           << "darkslategrey"
           << "darkturquoise"
           << "darkviolet"
           << "deeppink"
           << "deepskyblue"
           << "dimgray"
           << "dimgrey"
           << "dodgerblue"
           << "firebrick"
           << "floralwhite"
           << "forestgreen"
           << "gainsboro"
           << "ghostwhite"
           << "goldenrod"
           << "gold"
           << "greenyellow"
           << "grey"
           << "honeydew"
           << "hotpink"
           << "indianred"
           << "indigo"
           << "ivory"
           << "khaki"
           << "lavenderblush"
           << "lavender"
           << "lawngreen"
           << "lemonchiffon"
           << "lightblue"
           << "lightcoral"
           << "lightcyan"
           << "lightgoldenrodyellow"
           << "lightgray"
           << "lightgreen"
           << "lightgrey"
           << "lightpink"
           << "lightsalmon"
           << "lightseagreen"
           << "lightskyblue"
           << "lightslategray"
           << "lightslategrey"
           << "lightsteelblue"
           << "lightyellow"
           << "limegreen"
           << "linen"
           << "mediumaquamarine"
           << "mediumblue"
           << "mediumorchid"
           << "mediumpurple"
           << "mediumseagreen"
           << "mediumslateblue"
           << "mediumspringgreen"
           << "mediumturquoise"
           << "mediumvioletred"
           << "midnightblue"
           << "mintcream"
           << "mistyrose"
           << "moccasin"
           << "navajowhite"
           << "oldlace"
           << "olive"
           << "orangered"
           << "orchid"
           << "palegoldenrod"
           << "palegreen"
           << "paleturquoise"
           << "palevioletred"
           << "papayawhip"
           << "peachpuff"
           << "peru"
           << "pink"
           << "plum"
           << "powderblue"
           << "rosybrown"
           << "royalblue"
           << "saddlebrown"
           << "salmon"
           << "sandybrown"
           << "seagreen"
           << "seashell"
           << "sienna"
           << "skyblue"
           << "slateblue"
           << "slategray"
           << "slategrey"
           << "snow"
           << "springgreen"
           << "steelblue"
           << "tan"
           << "thistle"
           << "tomato"
           << "transparent"
           << "turquoise"
           << "violet"
           << "wheat"
           << "white"
           << "yellowgreen"
           << "rebeccapurple";
}

void
WidgetModel::initProperties()
{
  addProperty("alternate-background-color");
  addProperty("background");
  addProperty("background-color");
  addProperty("background-image");
  addProperty("background-repeat");
  addProperty("background-position");
  addProperty("background-attachment");
  addProperty("background-clip");
  addProperty("background-origin");
  addProperty("border");
  addProperty("border-top");
  addProperty("border-right");
  addProperty("border-bottom");
  addProperty("border-left");
  addProperty("border-color");
  addProperty("border-top-color");
  addProperty("border-right-color");
  addProperty("border-bottom-color");
  addProperty("border-left-color");
  addProperty("border-image");
  addProperty("border-radius");
  addProperty("border-top-left-radius");
  addProperty("border-top-right-radius");
  addProperty("border-bottom-right-radius");
  addProperty("border-bottom-left-radius");
  addProperty("border-style");
  addProperty("border-top-style");
  addProperty("border-right-style");
  addProperty("border-bottom-style");
  addProperty("border-left-style");
  addProperty("border-width");
  addProperty("border-top-width");
  addProperty("border-right-width");
  addProperty("border-bottom-width");
  addProperty("border-left-width");
  addProperty("bottom");
  addProperty("button-laout");
  addProperty("color");
  addProperty("dialogbuttonbox-buttons-have-icons");
  addProperty("font");
  addProperty("font-family");
  addProperty("font-size");
  addProperty("font-style");
  addProperty("font-weight");
  addProperty("gridline-color");
  addProperty("height");
  addProperty("icon");
  addProperty("icon-size");
  addProperty("image");
  addProperty("image-position");
  addProperty("left");
  addProperty("lineedit-password-character");
  addProperty("lineedit-password-mask-delay");
  addProperty("margin");
  addProperty("margin-top");
  addProperty("margin-right");
  addProperty("margin-bottom");
  addProperty("margin-left");
  addProperty("max-height");
  addProperty("max-width");
  addProperty("messagebox-text-interaction-flags");
  addProperty("min-height");
  addProperty("min-width");
  addProperty("opacity");
  addProperty("outline");
  addProperty("outline-color");
  addProperty("outline-offset");
  addProperty("outline-style");
  addProperty("outline-radius");
  addProperty("outline-bottom-left-radius");
  addProperty("outline-bottom-right-radius");
  addProperty("outline-top-left-radius");
  addProperty("outline-top-right-radius");
  addProperty("padding");
  addProperty("padding-top");
  addProperty("padding-right");
  addProperty("padding-bottom");
  addProperty("padding-left");
  addProperty("paint-alternating-row-colors-for-empty-area");
  addProperty("position");
  addProperty("right");
  addProperty("selection-background-color");
  addProperty("selection-color");
  addProperty("show-decoration-selected");
  addProperty("spacing");
  addProperty("subcontrol-origin");
  addProperty("subcontrol-position");
  addProperty("titlebar-show-tooltips-on-buttons");
  addProperty("widget-animation-duration");
  addProperty("text-align");
  addProperty("text-decoration");
  addProperty("top");
  addProperty("width");
  addProperty("-qt-background-role");
  addProperty("-qt-style-features");
  // I might as well add stylesheet stuff for this widget.
  addProperty("widget", true);
  addProperty("subcontrol", true);
  addProperty("subcontrolmarker", true);
  addProperty("pseudostate", true);
  addProperty("pseudostatemarker", true);
  addProperty("property", true);
  addProperty("propertymarker", true);
  addProperty("value", true);
  addProperty("startbrace", true);
  addProperty("endbrace", true);
  addProperty("bracematch", true);
  addProperty("comment", true);
  addProperty("bad", true);
}

void
WidgetModel::initSubControls()
{
  addSubControl("add-line", addControls(1, new QString("QScrollBar")));
  addSubControl("add-page", addControls(1, new QString("QScrollBar")));
  addSubControl("branch", addControls(1, new QString("QTreeView")));
  addSubControl("chunk", addControls(1, new QString("QProgressBar")));
  addSubControl(
    "close-button",
    addControls(2, new QString("QDockWidget"), new QString("QTabBar")));
  addSubControl("corner", addControls(1, new QString("QAbstractScrollArea")));
  addSubControl("down-arrow",
                addControls(4,
                            new QString("QComboBox"),
                            new QString("QHeaderView"),
                            new QString("QScrollBar"),
                            new QString("QSpinBox")));
  addSubControl(
    "down-button",
    addControls(2, new QString("QScrollBar"), new QString("QSpinBox")));
  addSubControl("drop-down", addControls(1, new QString("QComboBox")));
  addSubControl("float-button", addControls(1, new QString("QDockWidget")));
  addSubControl("groove", addControls(1, new QString("QSlider")));
  addSubControl("indicator",
                addControls(5,
                            new QString("QAbstractItemView"),
                            new QString("QCheckBox"),
                            new QString("QRadioButton"),
                            new QString("QMenu"),
                            new QString("QGroupBox")));
  addSubControl("handle",
                addControls(3,
                            new QString("QScrollBar"),
                            new QString("QSplitter"),
                            new QString("QSlider")));
  addSubControl(
    "icon",
    addControls(2, new QString("QAbstractItemView"), new QString("QMenu")));
  addSubControl("item",
                addControls(4,
                            new QString("QAbstractItemView"),
                            new QString("QMenuBar"),
                            new QString("QMenu"),
                            new QString("QStatusBar")));
  addSubControl("left-arrow", addControls(1, new QString("QScrollBar")));
  addSubControl("left-corner", addControls(1, new QString("QTabWidget")));
  addSubControl("menu-arrow", addControls(1, new QString("QToolButton")));
  addSubControl("menu-button", addControls(1, new QString("QToolButton")));
  addSubControl("menu-indicator", addControls(1, new QString("QPushButton")));
  addSubControl(
    "right-arrow",
    addControls(2, new QString("QMenu"), new QString("QScrollBar")));
  addSubControl("pane", addControls(1, new QString("QTabWidget")));
  addSubControl("right-corner", addControls(1, new QString("QTabWidget")));
  addSubControl("scroller",
                addControls(2, new QString("QMenu"), new QString("QTabBar")));
  addSubControl("section", addControls(1, new QString("QHeaderView")));
  addSubControl(
    "separator",
    addControls(2, new QString("QMenu"), new QString("QMainWindow")));
  addSubControl("sub-line", addControls(1, new QString("QScrollBar")));
  addSubControl("sub-page", addControls(1, new QString("QScrollBar")));
  addSubControl(
    "tab", addControls(2, new QString("QTabBar"), new QString("QToolBox")));
  addSubControl("tab-bar", addControls(1, new QString("QTabWidget")));
  addSubControl("tear", addControls(1, new QString("QTabBar")));
  addSubControl("tearoff", addControls(1, new QString("QMenu")));
  addSubControl("text", addControls(1, new QString("QAbstractItemView")));
  addSubControl(
    "title",
    addControls(2, new QString("QGroupBox"), new QString("QDockWidget")));
  addSubControl("up-arrow",
                addControls(3,
                            new QString("QHeaderView"),
                            new QString("QScrollBar"),
                            new QString("QSpinBox")));
  addSubControl("up-button", addControls(1, new QString("QSpinBox")));
}

void
WidgetModel::initPseudoStates()
{
  addPseudoState("active");
  addPseudoState("adjoins-item");
  addPseudoState("alternate");
  addPseudoState("bottom");
  addPseudoState("checked");
  addPseudoState("closable");
  addPseudoState("closed");
  addPseudoState("default");
  addPseudoState("disabled");
  addPseudoState("editable");
  addPseudoState("edit-focus");
  addPseudoState("enabled");
  addPseudoState("exclusive");
  addPseudoState("first");
  addPseudoState("flat");
  addPseudoState("floatable");
  addPseudoState("focus");
  addPseudoState("has-children");
  addPseudoState("has-siblings");
  addPseudoState("horizontal");
  addPseudoState("hover");
  addPseudoState("indeterminate");
  addPseudoState("last");
  addPseudoState("left");
  addPseudoState("maximized");
  addPseudoState("middle");
  addPseudoState("minimized");
  addPseudoState("movable");
  addPseudoState("no-frame");
  addPseudoState("non-exclusive");
  addPseudoState("off");
  addPseudoState("on");
  addPseudoState("only-one");
  addPseudoState("open");
  addPseudoState("next-selected");
  addPseudoState("pressed");
  addPseudoState("previous-selected");
  addPseudoState("read-only");
  addPseudoState("right");
  addPseudoState("selected");
  addPseudoState("top");
  addPseudoState("unchecked");
  addPseudoState("vertical");
  addPseudoState("window");
}

void
WidgetModel::initWidgetTree()
{
  m_root = new WidgetItem("QWidget", nullptr);
  m_widgets.insert("QWidget", m_root);

  addWidget("QWidget", "QAbstractButton");
  addWidget("QAbstractButton", "QPushButton");
  addWidget("QAbstractButton", "QCheckBox");
  addWidget("QAbstractButton", "QRadioButton");
  addWidget("QAbstractButton", "QToolButton");
  addWidget("QWidget", "QAbstractSlider");
  addWidget("QAbstractSlider", "QSlider");
  addWidget("QAbstractSlider", "QDial");
  addWidget("QAbstractSlider", "QScrollBar");
  addWidget("QWidget", "QAbstractSpinBox");
  addWidget("QAbstractSpinBox", "QSpinBox");
  addWidget("QAbstractSpinBox", "QDoubleSpinBox");
  addWidget("QAbstractSpinBox", "QDateTimeEdit");
  addWidget("QDateTimeEdit", "QDateEdit");
  addWidget("QDateTimeEdit", "QTimeEdit");
  addWidget("QWidget", "QDialog");
  addWidget("QWidget", "QComboBox");
  addWidget("QComboBox", "QFontComboBox");
  addWidget("QDialog", "QInputDialog");
  addWidget("QDialog", "QMessageBox");
  addWidget("QDialog", "QColorDialog");
  addWidget("QDialog", "QErrorMessage");
  addWidget("QDialog", "QFileDialog");
  addWidget("QDialog", "QFontDialog");
  addWidget("QDialog", "QProgressDialog");
  addWidget("QWidget", "QDialogButtonBox");
  addWidget("QWidget", "QFrame");
  addWidget("QFrame", "QAbstractScrollArea");
  addWidget("QAbstractScrollArea", "QAbstractItemView");
  addWidget("QAbstractScrollArea", "QGraphicsView");
  addWidget("QAbstractScrollArea", "QMdiArea");
  addWidget("QAbstractScrollArea", "QScrollArea");
  addWidget("QAbstractItemView", "QColumnView");
  addWidget("QAbstractItemView", "QHeaderView");
  addWidget("QAbstractItemView", "QListView");
  addWidget("QAbstractItemView", "QListWidget");
  addWidget("QAbstractItemView", "QUndoView");
  addWidget("QAbstractItemView", "QTreeView");
  addWidget("QTreeView", "QTreeWidget");
  addWidget("QAbstractItemView", "QTableView");
  addWidget("QTableView", "QTableWidget");
  addWidget("QAbstractScrollArea", "QPlainTextEdit");
  addWidget("QAbstractScrollArea", "QTextEdit");
  addWidget("QFrame", "QLabel");
  addWidget("QFrame", "QLCDNumber");
  addWidget("QFrame", "QSplitter");
  addWidget("QFrame", "QStackedWidget");
  addWidget("QFrame", "QToolBox");
  addWidget("QWidget", "QGroupBox");
  addWidget("QWidget", "QLineEdit");
  addWidget("QWidget", "QMainWindow");
  addWidget("QWidget", "QDockWidget");
  addWidget("QWidget", "QFocusFrame");
  addWidget("QWidget", "QKeySequenceEdit");
  addWidget("QWidget", "QMdiSubWindow");
  addWidget("QWidget", "QOpenGLWidget");
  addWidget("QWidget", "QProgressBar");
  addWidget("QWidget", "QQuickWidget");
  addWidget("QWidget", "QSizeGrip");
  addWidget("QWidget", "QSplashScreen");
  addWidget("QWidget", "QSplitterHandle");
  addWidget("QWidget", "QSvgWidget");
  addWidget("QWidget", "QWizardPage");
  //  addWidget("QWidget", "QDesktopWidget"); // Obsolete
  addWidget("QWidget", "QMenu");
  addWidget("QWidget", "QMenuBar");
  addWidget("QWidget", "QRubberBand");
  addWidget("QWidget", "QStatusBar");
  addWidget("QWidget", "QTabBar");
  addWidget("QWidget", "QTabWidget");
  addWidget("QWidget", "QToolBar");
  addWidget("QWidget", "QCalendarWidget");
  //  // extra widgets.
  //  addWidget("QPlainTextEdit", "StylesheetEdit", true);
}

void
WidgetModel::initAlignment()
{
  m_alignmentValues << "top"
                    << "bottom"
                    << "left"
                    << "right"
                    << "center";
}

void
WidgetModel::initGradients()
{
  m_gradient << "qlineargradient"
             << "qradialgradient"
             << "qconicalgradient";
}

void
WidgetModel::initAttachments()
{
  m_attachment << "scroll"
               << "fixed";
}

void
WidgetModel::initBorderStyle()
{
  m_borderStyle << "dashed"
                << "dot-dash"
                << "dot-dot-dash"
                << "dotted"
                << "double"
                << "groove"
                << "inset"
                << "outset"
                << "ridge"
                << "solid"
                << "none";
}

void
WidgetModel::initBorderImage()
{
  m_borderImage << "stretch"
                << "repeat";
}

void
WidgetModel::initFontStyle()
{
  m_fontStyle << "normal"
              << "italic"
              << "oblique";
}

void
WidgetModel::initFontWeight()
{
  m_fontWeight << "normal"
               << "bold"
               << "bolder"
               << "lighter"
               << "100"
               << "200"
               << "300"
               << "400"
               << "500"
               << "600"
               << "700"
               << "800"
               << "900";
}

void
WidgetModel::initIcon()
{
  m_icon << "disabled"
         << "active"
         << "normal"
         << "selected"
         << "on"
         << "off";
}

void
WidgetModel::initOrigin()
{
  m_origin << "dotted"
           << "solid"
           << "double"
           << "groove"
           << "ridge"
           << "inset"
           << "outset"
           << "none"
           << "hidden";
}

void
WidgetModel::initOutlineStyle()
{
  m_outlineStyle << "dotted"
                 << "solid"
                 << "double"
                 << "groove"
                 << "ridge"
                 << "inset"
                 << "outset"
                 << "none"
                 << "hidden";
}

void
WidgetModel::initOutlineColor()
{
  m_outlineColor = "invert";
}

void
WidgetModel::initOutlineWidth()
{
  m_outlineWidth << "thin"
                 << "medium"
                 << "thick";
}

void
WidgetModel::initPosition()
{
  m_position << "relative"
             << "absolute";
}

void
WidgetModel::initRepeat()
{
  m_repeat << "repeat-x"
           << "repeat-y"
           << "repeat"
           << "no-repeat";
}

void
WidgetModel::initTextDecoration()
{
  m_textDecoration << "none"
                   << "underline"
                   << "overline"
                   << "line-through";
}

void
WidgetModel::initAttributeMap()
{
  m_attributes.insert("alternate-background-color", Brush);
  m_attributes.insert("background", Background);
  m_attributes.insert("background-color", Brush);
  m_attributes.insert("background-image", Url);
  m_attributes.insert("background-repeat", Repeat);
  m_attributes.insert("background-position", Alignment);
  m_attributes.insert("background-clip", Origin);
  m_attributes.insert("background-origin", Origin);
  m_attributes.insert("border", Border);
  m_attributes.insert("border-top", Border);
  m_attributes.insert("border-left", Border);
  m_attributes.insert("border-right", Border);
  m_attributes.insert("border-bottom", Border);
  m_attributes.insert("border-color", BoxColors);
  m_attributes.insert("border-top-color", Brush);
  m_attributes.insert("border-right-color", Brush);
  m_attributes.insert("border-bottom-color", Brush);
  m_attributes.insert("border-left-color", Brush);
  m_attributes.insert("border-image", BorderImage);
  m_attributes.insert("border-radius", Radius);
  m_attributes.insert("border-top-left-radius", Radius);
  m_attributes.insert("border-top-right-radius", Radius);
  m_attributes.insert("border-bottom-right-radius", Radius);
  m_attributes.insert("border-bottom-left-radius", Radius);
  m_attributes.insert("border-style", BorderStyle);
  m_attributes.insert("border-top-style", BorderStyle);
  m_attributes.insert("border-right-style", BorderStyle);
  m_attributes.insert("border-bottom-style", BorderStyle);
  m_attributes.insert("border-left-style", BorderStyle);
  m_attributes.insert("border-width", BoxLengths);
  m_attributes.insert("border-top-width", Length);
  m_attributes.insert("border-right-width", Length);
  m_attributes.insert("border-bottom-width", Length);
  m_attributes.insert("border-left-width", Length);
  m_attributes.insert("bottom", Length);
  m_attributes.insert("button-layout", Number);
  m_attributes.insert("color", Brush);
  m_attributes.insert("dialogbuttonbox-buttons-have-icons", Brush);
  m_attributes.insert("font", Font);
  m_attributes.insert("font-family", String);
  m_attributes.insert("font-size", FontSize);
  m_attributes.insert("font-style", FontStyle);
  m_attributes.insert("font-weight", FontWeight);
  m_attributes.insert("gridline-color", Color);
  m_attributes.insert("height", Length);
  m_attributes.insert("icon", Url);
  m_attributes.insert("icon-size", Length);
  m_attributes.insert("image", Url);
  m_attributes.insert("image-position", Alignment);
  m_attributes.insert("left", Length);
  m_attributes.insert("lineedit-password-character", Number);
  m_attributes.insert("lineedit-password-mask-delay", Number);
  m_attributes.insert("margin", BoxLengths);
  m_attributes.insert("margin-top", Length);
  m_attributes.insert("margin-right", Length);
  m_attributes.insert("margin-bottom", Length);
  m_attributes.insert("margin-left", Length);
  m_attributes.insert("max-height", Length);
  m_attributes.insert("max-width", Length);
  m_attributes.insert("messagebox-text-interaction-flags", Number);
  m_attributes.insert("min-height", Length);
  m_attributes.insert("min-width", Length);
  m_attributes.insert("opacity", Length);
  m_attributes.insert("outline", Outline); // not known??
  m_attributes.insert("outline-color", Color);
  m_attributes.insert("outline-offset", Length);
  m_attributes.insert("outline-style", OutlineStyle);
  m_attributes.insert("outline-radius", OutlineRadius);
  m_attributes.insert("outline-bottom-left-radius", Radius);
  m_attributes.insert("outline-bottom-right-radius", Radius);
  m_attributes.insert("outline-top-left-radius", Radius);
  m_attributes.insert("outline-top-right-radius", Radius);
  m_attributes.insert("padding", BoxLengths);
  m_attributes.insert("padding-top", Length);
  m_attributes.insert("padding-right", Length);
  m_attributes.insert("padding-bottom", Length);
  m_attributes.insert("padding-left", Length);
  m_attributes.insert("paint-alternating-row-colors-for-empty-area", Bool);
  m_attributes.insert("position", Position);
  m_attributes.insert("right", Length);
  m_attributes.insert("selection-background-color", Brush);
  m_attributes.insert("selection-color", Brush);
  m_attributes.insert("show-decoration-selected", Boolean);
  m_attributes.insert("spacing", Length);
  m_attributes.insert("subcontrol-position", Alignment);
  m_attributes.insert("titlebar-show-tooltips-on-buttons", Bool);
  m_attributes.insert("widget-animation-duration", Number);
  m_attributes.insert("text-align", Alignment);
  m_attributes.insert("text-decoration", TextDecoration);
  m_attributes.insert("top", Length);
  m_attributes.insert("width", Length);
  m_attributes.insert("-qt-background-role", PaletteRole);
  m_attributes.insert("-qt-style-features", List);
}

WidgetModel::WidgetModel()
{
  initWidgetTree();
  initSubControls();
  initPseudoStates();
  initProperties();
  initColorNames();
  initPaletteRoles();
  initAlignment();
  initGradients();
  initAttachments();
  initBorderStyle();
  initBorderImage();
  initFontStyle();
  initFontWeight();
  initIcon();
  initOrigin();
  initOutlineStyle();
  initOutlineColor();
  initOutlineWidth();
  initPosition();
  initRepeat();
  initTextDecoration();
  initAttributeMap();
}

/*!
   \brief Takes a variable size list of strings add creates a QStringList odf
   them.
 */
QStringList
WidgetModel::addControls(int count, ...)
{
  va_list arguments;

  QStringList widgets;
  va_start(arguments, count);

  for (int i = 0; i < count; i++) {
    QString* currentString = va_arg(arguments, QString*);
    widgets << *currentString;
  }

  va_end(arguments);

  return widgets;
}

void
WidgetModel::addWidget(const QString& parentName,
                       const QString& name,
                       bool extraWidget)
{
  WidgetItem* parent = m_widgets.value(parentName);
  if (parent) {
    if (!m_widgets.contains(name)) {
      WidgetItem* item = new WidgetItem(name, parent);
      item->setExtraWidget(extraWidget);
      parent->addChild(item);
      m_widgets.insert(name, item);
    }
  }
}

void
WidgetModel::removeWidget(const QString& name)
{
  WidgetItem* parent = m_widgets.value(name)->parent();
  if (parent->isExtraWidget()) {
    m_widgets.remove(name);
    parent->removeChild(name);
  }
}

bool
WidgetModel::hasWidget(const QString& widget)
{
  return m_widgets.contains(widget);
}

WidgetItem*
WidgetModel::widgetItem(const QString& name)
{
  return m_widgets.value(name);
}

void
WidgetModel::addSubControl(const QString& control, QStringList widgets)
{
  if (m_subControls.contains(control)) {
    auto items = m_subControls.value(control);
    for (auto& name : widgets) {
      auto item = widgetItem(name);
      if (item && !item->hasSubControl(control)) {
        item->addSubcontrol(control);
        items.append(item);
      }
    }
    m_subControls.insert(control, items);
  } else {
    QList<WidgetItem*> items;
    for (auto& name : widgets) {
      auto item = widgetItem(name);
      item->addSubcontrol(control);
      items.append(item);
    }
    m_subControls.insert(control, items);
  }
}

void
WidgetModel::removeSubControl(const QString& control)
{
  if (m_subControls.contains(control)) {
    auto items = m_subControls.value(control);
    for (auto& item : items) {
      if (item && item->isExtraWidget()) {
        item->removeSubcontrol(control);
        items.removeOne(item);
      }
    }
    if (items.isEmpty()) {
      m_subControls.remove(control);
    } else {
      m_subControls.insert(control, items);
    }
  }
}

bool
WidgetModel::checkSubControlForWidget(const QString& widget,
                                      const QString& control)
{
  // first check direct widgets.
  auto items = m_subControls.value(control);
  if (!items.isEmpty()) {
    auto item = widgetItem(widget);
    if (items.contains(item)) {
      return true;
    }

    // then check for a subclassed widget.
    for (auto item : items) {
      while (item->parent() != nullptr) {
        if (item->name() == widget) {
          return true;
        }
      }
    }
  }

  return false;
}

bool
WidgetModel::isValidSubControlForWidget(const QString& widget,
                                        const QString& control)
{
  auto items = m_subControls.value(control);
  if (!items.isEmpty()) {
    for (auto item : items) {
      if (item->name() == widget) {
        return true;
      }
    }
  }

  return false;
}

PropertyStatus*
WidgetModel::isValidPropertyValueForProperty(const QString& propertyname,
                                             const QString& valuename)
{
  if (valuename.isEmpty()) {
    auto status =
      new PropertyStatus(PropertyStatus::PropertyStatus::BadPropertyValue);
    return status;
  }

  AttributeType propertyAttribute = m_attributes.value(propertyname);
  return checkPropertyValue(propertyAttribute, valuename);
}

bool
WidgetModel::containsSubControl(const QString& name)
{
  return m_subControls.contains(name.trimmed().toLower());
}

QStringList
WidgetModel::possibleWidgetsForSubControl(const QString& name)
{
  QStringList names;
  auto items = m_subControls.value(name);
  for (auto item : items) {
    names.append(recurseWidgetsForNames(item));
  }
  return names;
}

QStringList
WidgetModel::possibleSubControlsForWidget(const QString& widget)
{
  for (auto& item : m_widgets) {
    if (item->name() == widget) {
      return item->subControls();
    }
  }
  return QStringList();
}

void
WidgetModel::addPseudoState(const QString& state,
                            //                            QStringList widgets,
                            bool extraState)
{
  m_pseudoStates << state;
  m_pseudoStatesExtra << extraState;
}

void
WidgetModel::removePseudoState(const QString& state)
{
  auto i = m_pseudoStates.indexOf(state);
  if (m_pseudoStates.contains(state) && m_pseudoStatesExtra.at(i)) {
    m_pseudoStates.removeAt(i);
    m_pseudoStatesExtra.removeAt(i);
  }
}

bool
WidgetModel::containsPseudoState(const QString& name)
{
  return m_pseudoStates.contains(name);
}

// QStringList
// WidgetModel::pseudoStates()
//{
//  return m_pseudoStates;
//}

void
WidgetModel::addProperty(const QString& property, bool extraProperty)
{
  m_properties << property;
  m_propertiesExtra << extraProperty;
}

bool
WidgetModel::containsProperty(const QString& name)
{
  return m_properties.contains(name);
}

QStringList
WidgetModel::borderValues()
{
  QStringList values;
  values << m_borderStyle << m_borderImage << m_colors << m_paletteRoles
         << m_gradient;
  return values;
}

QMultiMap<int, QString>
WidgetModel::fuzzySearch(const QString& name, QStringList list) const
{
  QMultiMap<int, QString> matches;
  char* pattern = new char[name.length() + 1];
  strcpy(pattern, name.toStdString().c_str());

  int score = 0;

  for (auto& valueStr : list) {
    char* value = new char[valueStr.size() + 1];
    strcpy(value, valueStr.toStdString().c_str());

    if (fts::fuzzy_match(pattern, value, score)) {
      matches.insert(score, valueStr);
    }

    delete[] value;
  }

  delete[] pattern;

  return matches;
}

QMultiMap<int, QString>
WidgetModel::fuzzySearchWidgets(const QString& name)
{
  return fuzzySearch(name, m_widgets.keys());
}

QMultiMap<int, QString>
WidgetModel::fuzzySearchProperty(const QString& name)
{
  return fuzzySearch(name, m_properties);
}

QMultiMap<int, QString>
WidgetModel::fuzzySearchPropertyValue(const QString& name, const QString& value)
{
  QStringList list;
  auto attribute = m_attributes.value(name);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum,"
  switch (attribute) {
    case Alignment:
      return fuzzySearch(value, m_alignmentValues);

    case Attachment:
      return fuzzySearch(value, m_attachment);

    case Background: {
      QMap<int, QString> data;
      data.insert(fuzzySearch(value, m_colors));
      data.insert(fuzzySearch(value, m_paletteRoles));
      data.insert(fuzzySearch(value, m_gradient));
      return data;
    }

    case Border:
      list << m_borderStyle << m_borderImage << m_colors << m_paletteRoles
           << m_gradient;
      return fuzzySearch(value, list);

    case BorderImage:
      return fuzzySearch(value, m_borderImage);

    case BorderStyle:
      return fuzzySearch(value, m_borderStyle);

    case BoxColors:
    case Color:
      return fuzzySearch(value, m_colors);

    case Brush:
      list << m_colors << m_paletteRoles << m_gradient;
      return fuzzySearch(value, list);

    case Font:
      list << m_fontStyle << m_fontWeight;
      return fuzzySearch(value, list);

    case FontStyle:
      return fuzzySearch(value, m_fontStyle);

    case FontWeight:
      return fuzzySearch(value, m_fontWeight);

    case Gradient:
      return fuzzySearch(value, m_gradient);

    case Icon:
      return fuzzySearch(value, m_icon);

    case Origin:
      return fuzzySearch(value, m_origin);

    case Outline:
      list << m_outlineStyle << m_colors << m_outlineColor << m_outlineWidth;
      return fuzzySearch(value, list);

    case OutlineStyle:
      return fuzzySearch(value, m_outlineStyle);

    case PaletteRole:
      return fuzzySearch(value, m_paletteRoles);

    case Position:
      return fuzzySearch(value, m_position);

    case Repeat:
      return fuzzySearch(value, m_repeat);

    case TextDecoration:
      return fuzzySearch(value, m_textDecoration);

      //    case StylesheetEditGood:
      //      list << m_colors << "thin"
      //           << "extralight"
      //           << "light"
      //           << "normal"
      //           << "medium"
      //           << "demibold"
      //           << "bold"
      //           << "extrabold"
      //           << "black";
      //      return fuzzySearch(value, list);

      //    case StylesheetEditBad:
      //      list << m_colors << "thin"
      //           << "extralight"
      //           << "light"
      //           << "normal"
      //           << "medium"
      //           << "demibold"
      //           << "bold"
      //           << "extrabold"
      //           << "black"
      //           << "none"
      //           << "single"
      //           << "dash"
      //           << "dot"
      //           << "dashdot"
      //           << "dashdotdot"
      //           << "wave"
      //           << "spellcheck";
      //      return fuzzySearch(value, list);
  }
#pragma clang diagnostic pop
  return QMap<int, QString>();
}

QMultiMap<int, QString>
WidgetModel::fuzzySearchPseudoStates(const QString& name)
{
  return fuzzySearch(name, m_pseudoStates);
}

QMultiMap<int, QString>
WidgetModel::fuzzySearchSubControl(const QString& name)
{
  return fuzzySearch(name, m_subControls.keys());
}

AttributeType
WidgetModel::propertyValueAttribute(const QString& value)
{
  if (checkColor(value)->state == PropertyStatus::GoodPropertyValue) {
    return Color;

  } else if (checkLength(value)->state == PropertyStatus::GoodPropertyValue) {
    return Length;

  } else if (checkBorder(value)->state == PropertyStatus::GoodPropertyValue) {
    return Border;

  } else if (checkFont(value)->state == PropertyStatus::GoodPropertyValue) {
    return Font;

  } else if (checkFontWeight(value)->state ==
             PropertyStatus::GoodPropertyValue) {
    return FontWeight;

  } else if (checkRadius(value)->state == PropertyStatus::GoodPropertyValue) {
    return Radius;

  } else if (checkBrush(value)->state == PropertyStatus::GoodPropertyValue) {
    return Brush;

  } else if (checkFontStyle(value)->state ==
             PropertyStatus::GoodPropertyValue) {
    return FontStyle;

  } else if (checkFontSize(value)->state == PropertyStatus::GoodPropertyValue) {
    return FontSize;

  } else if (checkAlignment(value)->state ==
             PropertyStatus::GoodPropertyValue) {
    return Alignment;

  } else if (checkAttachment(value)->state ==
             PropertyStatus::GoodPropertyValue) {
    return Attachment;

  } else if (checkBackground(value)->state ==
             PropertyStatus::GoodPropertyValue) {
    return Background;

  } else if (checkBool(value)->state == PropertyStatus::GoodPropertyValue) {
    return Bool;

  } else if (checkBoolean(value)->state == PropertyStatus::GoodPropertyValue) {
    return Boolean;

  } else if (checkBorderImage(value)->state ==
             PropertyStatus::GoodPropertyValue) {
    return BorderImage;

  } else if (checkBorderStyle(value)->state ==
             PropertyStatus::GoodPropertyValue) {
    return BorderStyle;

  } else if (checkBoxColors(value)->state ==
             PropertyStatus::GoodPropertyValue) {
    return BoxColors;

  } else if (checkBoxLengths(value)->state ==
             PropertyStatus::GoodPropertyValue) {
    return BoxLengths;

  } else if (checkGradient(value)->state == PropertyStatus::GoodPropertyValue) {
    return Gradient;

  } else if (checkIcon(value)->state == PropertyStatus::GoodPropertyValue) {
    return Icon;

  } else if (checkNumber(value)->state == PropertyStatus::GoodPropertyValue) {
    return Number;

  } else if (checkOutline(value)->state == PropertyStatus::GoodPropertyValue) {
    return Outline;

  } else if (checkOrigin(value)->state == PropertyStatus::GoodPropertyValue) {
    return Origin;

  } else if (checkOutlineStyle(value)->state ==
             PropertyStatus::PropertyStatus::GoodPropertyValue) {
    return OutlineStyle;

  } else if (checkOutlineRadius(value)->state ==
             PropertyStatus::GoodPropertyValue) {
    return OutlineRadius;

  } else if (checkPaletteRole(value)->state ==
             PropertyStatus::GoodPropertyValue) {
    return PaletteRole;

  } else if (checkRepeat(value)->state == PropertyStatus::GoodPropertyValue) {
    return Repeat;

  } else if (checkUrl(value)->state == PropertyStatus::GoodPropertyValue) {
    return Url;

  } else if (checkPosition(value)->state == PropertyStatus::GoodPropertyValue) {
    return Position;

  } else if (checkTextDecoration(value)->state ==
             PropertyStatus::GoodPropertyValue) {
    return TextDecoration;

  } // TODO add property tests.
  /* else if (checkStylesheetEdit(value)==PropertyStatus::GoodPropertyValue) {
    return StylesheetEditGood;

  } else if (checkStylesheetEditBad(value)==PropertyStatus::GoodPropertyValue) {
    return StylesheetEditBad;
  }*/

  return NoAttributeValue;
}

QStringList
WidgetModel::eraseDuplicates(QStringList list)
{
  QStringList result;
  for (auto v : list) {
    if (!result.contains(v))
      result.append(v);
  }
  return result;
  // this doesn't seem to work with QList
  //  list->erase(std::unique(list->begin(), list->end()), list->end());
}

bool
WidgetModel::addCustomWidget(const QString& name, const QString& parent)
{
  if (hasWidget(parent)) {
    WidgetItem* parentItem = widgetItem(parent);
    auto item = new WidgetItem(name, parentItem);
    item->setExtraWidget(true);
    parentItem->addChild(item);
    return true;
  }
  return false;
}

bool
WidgetModel::addCustomWidgetPseudoStates(const QString& widgetName,
                                         QStringList states)
{
  if (hasWidget(widgetName)) {
    auto item = widgetItem(widgetName);
    states = eraseDuplicates(states);
    //    for (auto state : states) {
    m_customPseudoStates.insert(widgetName, states);
    //    }
    return true;
  }
  return false;
}

bool
WidgetModel::addCustomWidgetSubControls(const QString& widget,
                                        QStringList controls)
{
  if (m_customNames.contains(widget)) {
    controls = eraseDuplicates(controls);
    m_customSubControls.insert(widget, controls);
    return true;
  }
  return false;
}

bool
WidgetModel::addCustomWidgetProperties(const QString& widget,
                                       QStringList properties)
{
  if (m_customNames.contains(widget)) {
    properties = eraseDuplicates(properties);
    m_customProperties.insert(widget, properties);
    return true;
  }
  return false;
}

bool
WidgetModel::addCustomWidgetPropertyValue(const QString& widget,
                                          const QString& property,
                                          QString value)
{
  QMap<QString, QStringList> map;
  QStringList list;
  if (m_customValues.contains(widget)) {
    map = m_customValues.value(widget);
    if (map.contains(property)) {
      list = map.value(property);
    }
    list.append(value);
    list = eraseDuplicates(list);
    map.insert(property, list);
    m_customValues.insert(widget, map);
    return true;
  }
  return false;
}

bool
WidgetModel::addCustomWidgetPropertyValues(const QString& widget,
                                           const QString& property,
                                           QStringList values)
{
  if (m_customNames.contains(widget)) {
    QMap<QString, QStringList> map;
    QStringList list;
    if (m_customValues.contains(widget)) {
      map = m_customValues.value(widget);
      if (map.contains(property)) {
        list = map.value(property);
      }
      list.append(values);
    }
    list = eraseDuplicates(values);
    map.insert(property, list);
    m_customValues.insert(widget, map);
    return true;
  }
  return false;
}

QStringList
WidgetModel::recurseWidgetsForNames(WidgetItem* item)
{
  QStringList names;
  for (auto child : item->children()) {
    names.append(child->name());
    auto w = widgetItem(child->name());
    if (w && w->hasChildren()) {
      names.append(recurseWidgetsForNames(w));
    }
  }
  return names;
}

QString
Property::propertyName() const
{
  return m_propertyname;
}

void
Property::setPropertyName(const QString& propertyname)
{
  m_propertyname = propertyname;
}

AttributeTypes
Property::attributes() const
{
  return m_attributes;
}

void
Property::setAttributes(AttributeTypes attributes)
{
  m_attributes = attributes;
}

QStringList
DataStore::fontSizes()
{
  const QStringList fontSizes = { "6",  "7",  "8",  "9",  "10", "11",
                                  "12", "14", "16", "18", "20", "22",
                                  "24", "26", "28", "36", "48", "72" };
  return fontSizes;
}
