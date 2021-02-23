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
#include <QAbstractListModel>

#include <sm_widgets/labelledcombobox.h>
#include <sm_widgets/labelledspinbox.h>

class StylesheetEdit;
class StylesheetEditor;

class TypeModel : public QAbstractListModel
{

  // QAbstractItemModel interface
public:
  TypeModel(QObject* parent);
  QModelIndex index(int row,
                    int column,
                    const QModelIndex& parent) const override
  {}
  QModelIndex parent(const QModelIndex& child) const override {}
  int rowCount(const QModelIndex&) const override;
  int columnCount(const QModelIndex&) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  void setData(StylesheetEditor* editor);

private:
  StylesheetEditor* m_editor=nullptr;
  QString m_fontfamily;
  QStringList m_text;
  QList<QTextCharFormat> m_formats;

  void setData(const QString& text, QTextCharFormat format);
};

class ModifyFrame : public QWidget
{
  Q_OBJECT
public:
  ModifyFrame(QWidget* parent = nullptr);

  void setRow(int row, QTextCharFormat format);
private:
  void foregroundClicked(bool);
  void backgroundClicked(bool);
  void colorClicked(bool);
  void boldClicked(bool);
  void italicClicked(bool);
  void underlineTypeChanged(int index);
};

class ColorFontFrame : public QFrame
{
  Q_OBJECT
public:
  ColorFontFrame(StylesheetEditor* editor, QWidget* parent = nullptr);

private:
  StylesheetEditor* m_editor = nullptr;
  TypeModel *m_model;
  ModifyFrame *m_modify;

  void indexChanged(const QModelIndex &index);

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


protected:

private:
  QTabWidget* m_tabs;
  QDialogButtonBox* m_btnBox;
  StylesheetEditor* m_editor;

  void apply();
};

#endif // STYLESHEETEDITDIALOG_H
