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
#include "bookmarkarea.h"

#include "hoverwidget.h"
#include "stylesheetedit_p.h"
#include "stylesheetedit/labelledlineedit.h"
#include "stylesheetedit/labelledspinbox.h"

BookmarkArea::BookmarkArea(StylesheetEditor* editor)
  : QWidget(editor)
  , m_editor(editor)
  , m_foreSelected(QColor(0x80, 0x80, 0x80 /*"#808080"*/))
  , m_foreUnselected(QColor(0xC5, 0xC5, 0xC5 /*"#C5C5C5"*/))
  , m_back(QColor(0xEE, 0xEF, 0xEF /*"#EEEFEF"*/))
  , m_width(15)
  , m_left(0)
  , m_bookmarks(new QMap<int, BookmarkData*>())
{
  setMouseTracking(true);
}

QSize
BookmarkArea::sizeHint() const
{
  return QSize(m_editor->bookmarkAreaWidth(), 0);
}

QColor
BookmarkArea::foreSelected() const
{
  return m_foreSelected;
}

void
BookmarkArea::setForeSelected(const QColor& fore)
{
  m_foreSelected = fore;
}

QColor
BookmarkArea::foreUnselected() const
{
  return m_foreUnselected;
}

void
BookmarkArea::setForeUnselected(const QColor& fore)
{
  m_foreUnselected = fore;
}

QColor
BookmarkArea::back() const
{
  return m_back;
}

void
BookmarkArea::setBack(const QColor& back)
{
  m_back = back;
}

int
BookmarkArea::bookmarkAreaWidth() const
{
  return m_width;
}

void
BookmarkArea::setWidth(int width)
{
  m_width = width;
}

void
BookmarkArea::paintEvent(QPaintEvent* event)
{
  m_rect.setLeft(m_left);
  m_rect.setRight(m_left + event->rect().width());
  m_rect.setTop(event->rect().top());
  m_rect.setBottom(event->rect().bottom());

  QTextBlock block = m_editor->firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = qRound(m_editor->blockBoundingGeometry(block)
                     .translated(m_editor->contentOffset())
                     .top());
  int bottom = top + qRound(m_editor->blockBoundingRect(block).height());
  int height = m_editor->fontMetrics().height();
  double blockHeight = m_editor->blockBoundingRect(block).height();

  QPainter painter(this);
  painter.fillRect(m_rect, back());

  while (block.isValid() && top <= m_rect.bottom()) {
    if (block.isVisible() && bottom >= m_rect.top()) {
      int number = blockNumber + 1;

      if (m_bookmarks->contains(number)) {
        BookmarkData* data = m_bookmarks->value(number);
        painter.setPen(m_foreSelected);
        painter.drawText(
          0, top, width(), height, Qt::AlignRight, StylesheetEditor::m_arrow);
        data->rect = QRect(0, top, width(), height);
      }
    }

    block = block.next();
    top = bottom;
    bottom = top + qRound(blockHeight);
    ++blockNumber;
  }
}

void
BookmarkArea::mousePressEvent(QMouseEvent* event)
{

  auto addBookmarkAct = new QAction(tr("Add Bookmark"), this);
  auto removeBookmarkAct = new QAction(tr("Remove Bookmark"), this);
  auto clearBookmarksAct = new QAction(tr("Clear Bookmarks"), this);
  auto gotoBookmarkAct = new QAction(tr("Go To Bookmark"), this);
  auto editBookmarkAct = new QAction(tr("Edit Bookmark"), this);

  connect(addBookmarkAct,
          &QAction::triggered,
          this,
          &BookmarkArea::handleAddBookmark);
  connect(removeBookmarkAct,
          &QAction::triggered,
          this,
          &BookmarkArea::handleRemoveBookmark);
  connect(clearBookmarksAct,
          &QAction::triggered,
          this,
          &BookmarkArea::clearBookmarks);
  connect(gotoBookmarkAct,
          &QAction::triggered,
          this,
          &BookmarkArea::handleGotoBookmark);
  connect(editBookmarkAct,
          &QAction::triggered,
          this,
          &BookmarkArea::handleEditBookmark);

  auto menu = new QMenu(this);
  menu->addAction(addBookmarkAct);
  menu->addAction(editBookmarkAct);
  menu->addAction(removeBookmarkAct);
  menu->addSeparator();
  menu->addAction(gotoBookmarkAct);
  menu->addSeparator();
  menu->addAction(clearBookmarksAct);

  auto tc = m_editor->cursorForPosition(event->pos());
  auto lineNumber = m_editor->calculateLineNumber(tc);
  m_lineNumber = lineNumber;

  if (event->button() == Qt::RightButton) {
    if (hasBookmark(lineNumber)) {
      addBookmarkAct->setEnabled(false);
      editBookmarkAct->setEnabled(true);
      removeBookmarkAct->setEnabled(true);

    } else {
      addBookmarkAct->setEnabled(true);
      editBookmarkAct->setEnabled(false);
      removeBookmarkAct->setEnabled(false);
    }

    if (m_bookmarks->count() == 0) {
      clearBookmarksAct->setEnabled(false);
      gotoBookmarkAct->setEnabled(false);

    } else {
      clearBookmarksAct->setEnabled(true);
      gotoBookmarkAct->setEnabled(true);
    }

    menu->popup(mapToGlobal(event->pos()));
  } else {
    if (lineNumber >= 0) {
      m_editor->goToLine(lineNumber);
      emit m_editor->lineNumber(m_editor->currentLineNumber());
      emit m_editor->lineCount(m_editor->lineCount());
    } else
      QWidget::mousePressEvent(event);
  }
}

void
BookmarkArea::mouseMoveEvent(QMouseEvent* event)
{
  auto pos = event->pos();
  auto tc = m_editor->cursorForPosition(pos);
  auto lineNumber = m_editor->calculateLineNumber(tc);
  setToolTip(QString());
  if (lineNumber > 0 && lineNumber <= m_editor->lineCount()) {
    auto bm = m_bookmarks->value(lineNumber);
    if (bm) {
      if (bm->text.isEmpty())
        setToolTip(tr("Line %1").arg(lineNumber));
      else
        setToolTip(tr("Line %1: %2")
                     .arg(lineNumber)
                     .arg(m_bookmarks->value(lineNumber)->text));
    }
  }
}

void
BookmarkArea::handleAddBookmark(bool)
{
  if (!m_bookmarks->contains(m_lineNumber)) {
    insertBookmark(m_lineNumber);
  }
}

void
BookmarkArea::handleRemoveBookmark(bool)
{
  if (m_bookmarks->contains(m_lineNumber)) {
    removeBookmark(m_lineNumber);
  }
}

void
BookmarkArea::handleEditBookmark(bool)
{
  if (m_bookmarks->contains(m_lineNumber)) {
    editBookmark(m_lineNumber);
  }
}

void
BookmarkArea::handleGotoBookmark()
{}

void
BookmarkArea::handleClearBookmarks(bool)
{
  clearBookmarks();
}

// void BookmarkArea::drawHoverWidget(QPoint pos, QString text)
//{

//  m_hoverWidget->setText(text);
//  m_hoverWidget->setGeometry(
//    pos.x() + 40, pos.y(), m_hoverWidget->width(), m_hoverWidget->height());
//  m_hoverWidget->show();
//}

// void BookmarkArea::hideHoverWidget()
//{
//  if (m_hoverWidget) {
//    m_hoverWidget->hide();
//  }
//}

// int BookmarkArea::left() const
//{
//  return m_left;
//}

// void BookmarkArea::setLeft(int left)
//{
//  m_left = left;
//}

int
BookmarkArea::isIn(QPoint pos)
{
  if (m_rect.isValid()) {
    if (pos.x() >= m_rect.left() && pos.x() <= m_rect.right() &&
        pos.y() >= m_rect.top() && pos.y() <= m_rect.bottom()) {
      auto keys = m_bookmarks->keys();
      for (auto& key : keys) {
        BookmarkData* data = m_bookmarks->value(key);

        if (data->rect.contains(pos)) {
          return key;
        }
      }
    }
  }

  return -1;
}

QMap<int, BookmarkData*>*
BookmarkArea::bookmarks()
{
  return m_bookmarks;
}

void
BookmarkArea::setBookmarks(QMap<int, BookmarkData*>* bookmarks)
{
  if (m_bookmarks && !m_bookmarks->isEmpty()) {
    qDeleteAll(*m_bookmarks);
    delete m_bookmarks;
  }

  auto keys = bookmarks->keys();
  for (auto& key : keys) {
    if (key <= m_editor->lineCount()) {
      m_bookmarks->insert(key, bookmarks->value(key));
    }
  }
  update();
}

void
BookmarkArea::insertBookmark(int bookmark, const QString& text)
{
  if (m_bookmarks->contains(bookmark) && hasBookmarkText(bookmark)) {
    m_oldBookmarks.insert(bookmark, bookmarkText(bookmark));

  } else {
    if (bookmark > 0 && bookmark <= m_editor->lineCount()) {
      BookmarkData* data = new BookmarkData();
      data->text = text;
      m_bookmarks->insert(bookmark, data);
    }
  }

  update();
}

void
BookmarkArea::toggleBookmark(int bookmark)
{
  if (m_bookmarks->contains(bookmark)) {
    removeBookmark(bookmark);

  } else {
    if (m_oldBookmarks.contains(bookmark)) {
      insertBookmark(bookmark, m_oldBookmarks.value(bookmark));

    } else {
      insertBookmark(bookmark);
    }
  }
}

void
BookmarkArea::removeBookmark(int bookmark)
{
  if (m_bookmarks->contains(bookmark)) {
    m_oldBookmarks.insert(bookmark, m_bookmarks->value(bookmark)->text);
    m_bookmarks->remove(bookmark);
    update();
  }
}

void
BookmarkArea::editBookmark(int lineNumber)
{
  auto lineCount = m_editor->lineCount();
  if (lineNumber > 0 && lineNumber < lineCount) {
    QString text = bookmarkText(lineNumber);

    BookmarkEditDialog dlg(this);
    dlg.setText(text);
    dlg.setLineNumber(lineNumber);
    auto result = dlg.exec();

    if (result == QDialog::Accepted) {
      auto ln = dlg.lineNumber();
      removeBookmark(lineNumber);
      insertBookmark(ln, dlg.text());
    }
  }
}

void
BookmarkArea::clearBookmarks()
{
  m_bookmarks->clear();
  update();
}

bool
BookmarkArea::hasBookmark(int bookmark)
{
  return m_bookmarks->contains(bookmark);
}

bool
BookmarkArea::hasBookmarkText(int bookmark)
{
  return !m_bookmarks->value(bookmark)->text.isEmpty();
}

QString
BookmarkArea::bookmarkText(int bookmark)
{
  QString text = m_bookmarks->value(bookmark)->text;
  text = (text.isEmpty() ? tr("Bookmark") : text);
  return text;
}

int
BookmarkArea::count()
{
  return m_bookmarks->size();
}

BookmarkModel::BookmarkModel(QMap<int, BookmarkData*>* bookmarks)
  : QAbstractTableModel()
  , m_bookmarks(bookmarks)
{}

int
BookmarkModel::columnCount(const QModelIndex&) const
{
  return 2;
}

int
BookmarkModel::rowCount(const QModelIndex&) const
{
  return m_bookmarks->size();
}

QVariant
BookmarkModel::data(const QModelIndex& index, int role) const
{
  if (index.isValid() && role == Qt::DisplayRole) {
    switch (index.column()) {
      case 0:
        return m_bookmarks->keys().at(index.row());

      case 1: {
        int i = m_bookmarks->keys().at(index.row());
        return m_bookmarks->value(i)->text;
      }
    }
  }

  return QVariant();
}

Qt::ItemFlags
BookmarkModel::flags(const QModelIndex& index) const
{
  if (index.isValid()) {
    return (QAbstractTableModel::flags(index));
  }

  return Qt::NoItemFlags;
}

QVariant
BookmarkModel::headerData(int section,
                          Qt::Orientation orientation,
                          int /*role*/) const
{
  if (orientation == Qt::Horizontal) {
    if (section == 0) {
      return tr("Bookmark");

    } else {
      return tr("Message");
    }
  }

  return QVariant();
}

GoToBookmarkDialog::GoToBookmarkDialog(QMap<int, BookmarkData*>* bookmarks,
                                       QWidget* parent)
  : QDialog(parent)
  , m_bookmark(-1)
  , m_text(QString())
{
  QVBoxLayout* layout = new QVBoxLayout;
  setLayout(layout);

  m_group = new QGroupBox(tr("Bookmarks"), this);
  //  m_group->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_group);

  QVBoxLayout* grpLayout = new QVBoxLayout;
  grpLayout->setContentsMargins(0, 0, 0, 0);
  m_group->setLayout(grpLayout);

  BookmarkModel* bookmarkModel = new BookmarkModel(bookmarks);
  m_bookmarkView = new QTableView(this);
  m_bookmarkView->setModel(bookmarkModel);
  m_bookmarkView->verticalHeader()->hide();
  m_bookmarkView->horizontalHeader()->setVisible(true);
  m_bookmarkView->horizontalHeader()->setStretchLastSection(true);
  grpLayout->addWidget(m_bookmarkView);
  connect(m_bookmarkView,
          &QTableView::clicked,
          this,
          &GoToBookmarkDialog::handleClicked);

  m_buttonBox =
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  layout->addWidget(m_buttonBox);

  connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

int
GoToBookmarkDialog::bookmark()
{
  return m_bookmark;
}

QString
GoToBookmarkDialog::text()
{
  return m_text;
}

void
GoToBookmarkDialog::handleClicked(const QModelIndex& index)
{
  QModelIndex i0 = m_bookmarkView->model()->index(index.row(), 0);
  QModelIndex i1 = m_bookmarkView->model()->index(index.row(), 1);

  if (i0.isValid()) {
    m_bookmark = m_bookmarkView->model()->data(i0).toInt();
  }

  if (i1.isValid()) {
    m_text = m_bookmarkView->model()->data(i1).toString();
  }
}

BookmarkEditDialog::BookmarkEditDialog(QWidget* parent)
  : QDialog(parent)
{
  QVBoxLayout* layout = new QVBoxLayout;
  setLayout(layout);

  m_textEdit = new LabelledLineEdit(tr("Note Text:"), this);
  layout->addWidget(m_textEdit);
  m_linenumberEdit = new LabelledSpinBox(tr("Line Number"), this);
  layout->addWidget(m_linenumberEdit);

  m_buttonBox =
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  layout->addWidget(m_buttonBox);

  connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void
BookmarkEditDialog::setText(const QString& text)
{
  m_textEdit->setText(text);
}

QString
BookmarkEditDialog::text()
{
  return m_textEdit->text();
}

void
BookmarkEditDialog::setLineNumber(int linenumber)
{
  m_linenumberEdit->setValue(linenumber);
}

int
BookmarkEditDialog::lineNumber()
{
  return m_linenumberEdit->value();
}

/// \endcond DO_NOT_DOCUMENT
