#include <QtWidgets>
#include <QHBoxLayout>

#include "mainwindow.h"

/* Libc includes */
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	remote_editor = new QTextEdit;
	local_editor = new QTextEdit;

	setupEditor(remote_editor);
	setupEditor(local_editor);
	remote_editor->setReadOnly(true);

	QPushButton *button = new QPushButton("Submit", this);
	connect(button, &QAbstractButton::clicked, this, &MainWindow::submit);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(remote_editor);
	layout->addWidget(local_editor);
	layout->addWidget(button);

	QWidget *centralWidget = new QWidget;
	centralWidget->setLayout(layout);
	setCentralWidget(centralWidget);

	new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this, SLOT(submit()));
	new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return), this, SLOT(submit()));
}

void MainWindow::setupEditor(QTextEdit *editor)
{
	QFont font;
	font.setFamily("Courier");
	font.setFixedPitch(true);
	font.setPointSize(10);

	editor->setFont(font);

	//highlighter = new Highlighter(editor->document());
}

void MainWindow::load()
{
	char buf[4096];

	ssize_t n = read(0, buf, sizeof(buf)-1);
	buf[n] = 0;
	remote_editor->setPlainText(QString(buf));
}

void MainWindow::submit()
{
	QByteArray b = local_editor->toPlainText().toLocal8Bit();
	if (b.size() > 0)
		write(1, b.data(), b.size());
	load();
}
