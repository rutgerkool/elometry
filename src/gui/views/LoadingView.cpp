#include "gui/views/LoadingView.h"
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QFont>

LoadingView::LoadingView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void LoadingView::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);
    mainLayout->setAlignment(Qt::AlignCenter);

    appNameLabel = new QLabel("Elometry", this);
    QFont headerFont = appNameLabel->font();
    headerFont.setPointSize(24);
    headerFont.setBold(true);
    appNameLabel->setFont(headerFont);
    appNameLabel->setStyleSheet("color: #0078d4;");
    appNameLabel->setAlignment(Qt::AlignCenter);
    
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setMinimumWidth(300);
    progressBar->setTextVisible(true);
    
    statusLabel = new QLabel("Initializing...", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    QFont statusFont = statusLabel->font();
    statusFont.setPointSize(10);
    statusLabel->setFont(statusFont);
    
    mainLayout->addStretch();
    mainLayout->addWidget(appNameLabel);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(statusLabel);
    mainLayout->addStretch();
    
    setWindowTitle("Loading Elometry");
    setMinimumSize(400, 250);
}

void LoadingView::updateStatus(const QString& status)
{
    statusLabel->setText(status);
}

void LoadingView::updateProgress(int value)
{
    progressBar->setValue(value);
}
