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
#ifndef STYLESHEETEDITDIALOG_H
#define STYLESHEETEDITDIALOG_H

#include <QAbstractListModel>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFontComboBox>
#include <QFontDatabase>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextCharFormat>
#include <QVBoxLayout>

#include <sm_widgets/labelledcombobox.h>
#include <sm_widgets/labelledspinbox.h>

class StylesheetEdit;
class StylesheetEditor;

class TypeModel : public QAbstractListModel
{

  // QAbstractItemModel interface
public:
  TypeModel(QObject* parent);
  int rowCount(const QModelIndex&) const override;
  int columnCount(const QModelIndex&) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  void populate(StylesheetEditor* editor);
  QTextCharFormat format(int row);
  void setFormat(int row, QTextCharFormat format);

  QList<QTextCharFormat> formats() const;

private:
  StylesheetEditor* m_editor = nullptr;
  QString m_fontfamily;
  QStringList m_text;
  QList<QTextCharFormat> m_formats;

  void populateItem(const QString& text, QTextCharFormat format);
};

class ModifyFrame : public QWidget
{
  Q_OBJECT
public:
  ModifyFrame(QWidget* parent = nullptr);

  void setRow(int row, QTextCharFormat format);

signals:
  void rowChanged(int row, QTextCharFormat format);

private:
  int m_row;
  QTextCharFormat m_format;
  QPushButton *m_foregroundBtn, *m_backgroundBtn, *m_underlineColorBtn;
  QCheckBox *m_boldBox, *m_italicBox;
  QComboBox* m_underlineType;

  void foregroundClicked(bool);
  void backgroundClicked(bool);
  void colorClicked(bool);
  void boldClicked(bool checked);
  void italicClicked(bool checked);
  void underlineTypeChanged(int index);
};

class ColorFontFrame : public QFrame
{
  Q_OBJECT
public:
  ColorFontFrame(StylesheetEditor* editor, QWidget* parent = nullptr);

  QList<QTextCharFormat> formats() const;

//signals:
//  void formatChanged(int row, QTextCharFormat& format);

private:
  StylesheetEditor* m_editor = nullptr;
  TypeModel* m_model;
  ModifyFrame* m_modify;
  QListView* m_typeList;
  StylesheetEdit* m_display;

  void indexChanged(const QModelIndex& index);
  void rowChanged(int row, QTextCharFormat format);

  static const QString DISPLAY;
};

class StylesheetEditDialog : public QDialog
{
  Q_OBJECT
public:
  explicit StylesheetEditDialog(StylesheetEditor* editor,
                                QWidget* parent = nullptr);

  //  StylesheetEdit *editor() const;
  void setEditor(StylesheetEdit* editor);

  QList<QTextCharFormat> formats() const;

protected:
private:
  QTabWidget* m_tabs;
  QDialogButtonBox* m_btnBox;
  StylesheetEditor* m_editor;
  ColorFontFrame* m_colorFontFrame;

  void apply();
};

#endif // STYLESHEETEDITDIALOG_H
