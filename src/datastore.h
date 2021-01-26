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
#ifndef DATASTORE_H
#define DATASTORE_H

#include <QFile>
#include <QIcon>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QStack>
#include <QTextCursor>

#include "common.h"

#define FTS_FUZZY_MATCH_IMPLEMENTATION
#include "fts_fuzzy_match.h"

class StylesheetEditor;
class StylesheetData;
class Node;
class WidgetNode;
//class StartBraceNode;
//class EndBraceNode;

class DataStore : public QObject
{
  Q_OBJECT
public:
  explicit DataStore(QObject* parent = nullptr);
  ~DataStore();

  void addWidget(const QString& widget);
  void removeWidget(const QString& widget);
  bool containsWidget(const QString& name);
  QMultiMap<int, QString> fuzzySearchWidgets(const QString& name);
  bool containsProperty(const QString& name);
  QMultiMap<int, QString> fuzzySearchProperty(const QString& name);
  QMultiMap<int, QString> fuzzySearchPropertyValue(const QString& name,
                                                   const QString& value);
  bool containsStylesheetProperty(const QString& name);
  bool containsPseudoState(const QString& name);
  QMultiMap<int, QString> fuzzySearchPseudoStates(const QString& name);
  bool containsSubControl(const QString& name);
  bool isValidSubControlForWidget(const QString&widget, const QString& subcontrol);
  QMultiMap<int, QString> fuzzySearchSubControl(const QString& name);
  QMultiMap<int, QString> fuzzySearchSubControlForWidget(const QString& widget,const QString& name);

  bool ifValidStylesheetValue(const QString& propertyname,
                              const QString& valuename,
                              StylesheetData* data);
  bool isValidPropertyValueForProperty(const QString& propertyname,
                                       const QString& value);
  AttributeType propertyValueAttribute(const QString& value);

  //! Returns the names of all widgets for which this sub-control is valid.
  QStringList possibleSubControlWidgets(const QString& name);

  void addSubControl(const QString& control, const QString& widget);
  void addSubControl(const QString& control, QStringList& widgets);
  void removeSubControl(const QString& control);
  void addPseudoState(const QString& state);
  void removePseudoState(const QString& state);

  QMap<QTextCursor, Node*> nodes();
  void insertNode(QTextCursor cursor, Node* node);
  bool isNodesEmpty();
  void clearNodes();
  void setNodes(QMap<QTextCursor, Node*> nodes);

  int braceCount();
  void setBraceCount(int value);
  void incrementBraceCount();
  bool decrementBraceCount();

  int maxSuggestionCount();
  void setMaxSuggestionCount(int maxSuggestionCount);

  bool hasSuggestion();
  void setHasSuggestion(bool suggestion);

  bool isManualMove();
  void setManualMove(bool manualMove);

  Node* currentNode();
  void setCurrentWidget(WidgetNode* value);
  bool isCurrentWidget(WidgetNode* node);

  QTextCursor currentCursor();
  void setCurrentCursor(const QTextCursor& currentCursor);

  QIcon invalidIcon() const;
  QIcon validIcon() const;
  QIcon addDColonIcon() const;
  QIcon addSemiColonIcon() const;
  QIcon addColonIcon() const;
  QIcon badSemiColonIcon() const;
  QIcon badColonIcon() const;
  QIcon badDColonIcon() const;
  QIcon noIcon() const;
  QIcon fuzzyIcon() const;

signals:
  void finished();

private:
  const QIcon m_invalidIcon, m_validIcon, m_addSemiColonIcon, m_addDColonIcon, m_addColonIcon,
    m_badSColonIcon, m_badColonIcon, m_badDColonIcon, m_noIcon, m_fuzzyIcon;
  QMutex m_mutex;
  QStringList m_widgets;
  QStringList m_colors;
  QStringList m_attributeNames;
  QStringList m_properties;
  QStringList m_pseudoStates;
  QStringList m_possibleWidgets;
  QStringList m_StylesheetProperties;
  QStringList m_alignmentValues;
  QStringList m_paletteRoles, m_gradient, m_attachment, m_borderStyle,
    m_borderImage, m_fontStyle, m_fontWeight, m_icon, m_origin, m_outlineStyle,
    m_outlineWidth, m_position, m_repeat, m_textDecoration;
  QString m_outlineColor;

  QMap<QString, QStringList> m_subControls;
  QMap<QString, AttributeType> m_attributes;
  QMap<QString, AttributeType> m_stylesheetAttributes;

  QMap<QTextCursor, Node*> m_nodes;
//  QList<StartBraceNode*> m_startbraces;
//  QList<EndBraceNode*> m_endbraces;
  int m_braceCount;
  bool m_manualMove, m_hasSuggestion;
  QTextCursor m_currentCursor;
  Node* m_currentNode;
  int m_maxSuggestionCount;
//  QStack<StartBraceNode*> m_braceStack;

  bool isBraceCountZero();
//  void pushStartBrace(StartBraceNode* startbrace);
//  void pushEndBrace(EndBraceNode* endbrace);

  bool checkPropertyValue(AttributeType propertyAttribute,
                          const QString& valuename,
                          StylesheetData* data = nullptr);
  bool checkAlignment(const QString& value) const;
  bool checkAttachment(const QString& value) const;
  bool checkBackground(const QString& value) const;
  bool checkBool(const QString& value) const;
  bool checkBoolean(const QString& value) const;
  bool checkBorder(const QString& value) const;
  bool checkBorderImage(const QString& value) const;
  bool checkBorderStyle(const QString& value) const;
  bool checkBoxColors(const QString& value) const;
  bool checkBoxLengths(const QString& value) const;
  bool checkBrush(const QString& value) const;
  bool checkColor(const QString& value) const;
  bool checkFontStyle(const QString& value) const;
  bool checkFont(const QString& value) const;
  bool checkFontSize(const QString& value) const;
  bool checkFontWeight(const QString& value) const;
  bool checkGradient(const QString& value) const;
  bool checkIcon(const QString& value) const;
  bool checkLength(const QString& value) const;
  bool checkNumber(const QString& value) const;
  bool checkOutline(const QString& value) const;
  bool checkOrigin(const QString& value) const;
  bool checkOutlineStyle(const QString& value) const;
  bool checkOutlineColor(const QString& value) const;
  bool checkOutlineWidth(const QString& value) const;
  bool checkOutlineOffset(const QString& value) const;
  bool checkOutlineRadius(const QString& value) const;
  bool checkPaletteRole(const QString& value) const;
  bool checkRadius(const QString& value) const;
  bool checkRepeat(const QString& value) const;
  bool checkUrl(const QString& value) const;
  bool checkPosition(const QString& value) const;
  bool checkTextDecoration(const QString& value) const;
  //
  bool checkStylesheetEdit(const QString& value,
                           StylesheetData* data = nullptr) const;
  bool checkStylesheetEditBad(const QString& value,
                              StylesheetData* data = nullptr) const;
  bool checkStylesheetFontWeight(const QString& value,
                                 StylesheetData* data) const;

  QMap<QString, QStringList> initialiseSubControlMap();
  QStringList initialiseWidgetList();
  QStringList initialisePropertyList();
  QStringList initialisePseudoStateList();
  QMap<QString, AttributeType> initialiseAttributeMap();
  QStringList initialiseStylesheetProperties();
  QMap<QString, AttributeType> initialiseStylesheetMap();
  QStringList initialiseColorList();

  QStringList addControls(int count, ...);

  QMultiMap<int, QString> fuzzySearch(const QString& name, QStringList list);
  //  QMap<int, QString> fuzzyTestColorNames(const QString &value);
  QMap<int, QString> fuzzyTestBrush(const QString& value);
};

#endif // DATASTORE_H
