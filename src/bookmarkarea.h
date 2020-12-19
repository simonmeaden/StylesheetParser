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
#ifndef BOOKMARKAREA_H
#define BOOKMARKAREA_H

#include <QAbstractTableModel>
#include <QAction>
#include <QColor>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHeaderView>
#include <QMenu>
#include <QRect>
#include <QTableView>
#include <QVariant>
#include <QWidget>

class LabelledSpinBox;
class LabelledLineEdit;
class HoverWidget;
class StylesheetEditor;

class BookmarkData
{
public:
  BookmarkData(QString str = QString()) { text = str; }
  QString text;
  QRect rect;
};

class BookmarkEditDialog : public QDialog
{
  Q_OBJECT
public:
  explicit BookmarkEditDialog(QWidget* parent = nullptr);

  void setText(const QString& text);
  QString text();
  void setLineNumber(int linenumber);
  int lineNumber();

private:
  LabelledSpinBox* m_linenumberEdit;
  LabelledLineEdit* m_textEdit;
  QDialogButtonBox* m_buttonBox;
};

class BookmarkModel : public QAbstractTableModel
{
public:
  explicit BookmarkModel(QMap<int, BookmarkData*>* bookmarks);
  int columnCount(const QModelIndex& = QModelIndex()) const;
  int rowCount(const QModelIndex& = QModelIndex()) const;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int = Qt::DisplayRole) const;

private:
  QMap<int, BookmarkData*>* m_bookmarks;
};

class GoToBookmarkDialog : public QDialog
{
  Q_OBJECT
public:
  explicit GoToBookmarkDialog(QMap<int, BookmarkData*>* bookmarks,
                              QWidget* parent = nullptr);

  int bookmark();
  QString text();

private:
  QGroupBox* m_group;
  QTableView* m_bookmarkView;
  QDialogButtonBox* m_buttonBox;
  int m_bookmark;
  QString m_text;

  void handleClicked(const QModelIndex& index);
};

class BookmarkArea : public QWidget
{

public:
  BookmarkArea(StylesheetEditor* parent = nullptr);

  QSize sizeHint() const override;

  QColor foreSelected() const;
  void setForeSelected(const QColor& fore);
  QColor foreUnselected() const;
  void setForeUnselected(const QColor& fore);
  QColor back() const;
  void setBack(const QColor& back);

  int bookmarkAreaWidth() const;
  void setWidth(int width);

  //  int left() const;
  //  void setLeft(int left);

  int isIn(QPoint pos);

  QMap<int, BookmarkData*>* bookmarks();
  void setBookmarks(QMap<int, BookmarkData*>* bookmarks);
  void insertBookmark(int bookmark,
                      const QString& text = QString());
  void toggleBookmark(int bookmark);
  void removeBookmark(int bookmark);
  void editBookmark(int lineNumber);
  void clearBookmarks();
  bool hasBookmark(int bookmark);
  bool hasBookmarkText(int bookmark);
  QString bookmarkText(int bookmark);
  int count();

protected:
  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  //  void contextMenuEvent(QContextMenuEvent* event);
  void leaveEvent(QEvent* event) override;

  void handleAddBookmark(bool);
  void handleRemoveBookmark(bool);
  void handleEditBookmark(bool);
  void handleGotoBookmark();
  void handleClearBookmarks(bool);

private:
  StylesheetEditor* m_editor;
  QColor m_foreSelected, m_foreUnselected, m_back;
  QFont::Weight m_weight;
  int m_width, m_left;
  QMap<int, BookmarkData*>* m_bookmarks;
  QMap<int, QString> m_oldBookmarks;
  QRect m_rect;
  HoverWidget* m_hoverWidget;
  int m_lineNumber;

  //  void drawHoverWidget(QPoint pos, QString text);
  //  void hideHoverWidget();
};

#endif // BOOKMARKAREA_H
