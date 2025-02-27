#include "gui/views/LoadingView.h"
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QFont>
#include <QTimer>
#include <QtWidgets/QApplication>
#include <QEasingCurve>

LoadingView::LoadingView(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setupAnimations();
}

void LoadingView::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    mainLayout->setSpacing(30);
    mainLayout->setAlignment(Qt::AlignCenter);

    appNameLabel = new QLabel("Elometry", this);
    QFont headerFont("Segoe UI", 36);
    headerFont.setBold(true);
    appNameLabel->setFont(headerFont);
    appNameLabel->setStyleSheet("color: #0c7bb3;");
    appNameLabel->setAlignment(Qt::AlignCenter);
    
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setMinimumWidth(400);
    progressBar->setMinimumHeight(10);
    progressBar->setMaximumHeight(10);
    progressBar->setTextVisible(false);
    
    statusOpacityEffect = new QGraphicsOpacityEffect(this);
    statusOpacityEffect->setOpacity(1.0);
    
    statusLabel = new QLabel("Initializing", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setGraphicsEffect(statusOpacityEffect);
    QFont statusFont("Segoe UI", 12);
    statusLabel->setFont(statusFont);
    statusLabel->setStyleSheet("color: #a0a0a0;");
    
    mainLayout->addStretch();
    mainLayout->addWidget(appNameLabel);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(statusLabel);
    mainLayout->addStretch();
    
    setWindowTitle("Loading Elometry");
    setMinimumSize(500, 350);
}

void LoadingView::setupAnimations()
{
    progressAnimation = new QPropertyAnimation(progressBar, "value");
    progressAnimation->setDuration(300);
    progressAnimation->setEasingCurve(QEasingCurve::OutQuad);
    
    statusOpacityAnimation = new QPropertyAnimation(statusOpacityEffect, "opacity");
    statusOpacityAnimation->setDuration(250);
    statusOpacityAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    
    appNameAnimation = new QPropertyAnimation(appNameLabel, "geometry");
    appNameAnimation->setDuration(800);
    appNameAnimation->setEasingCurve(QEasingCurve::OutBack);
}

void LoadingView::updateStatus(const QString& status)
{
    statusOpacityAnimation->setStartValue(1.0);
    statusOpacityAnimation->setEndValue(0.0);
    
    connect(statusOpacityAnimation, &QPropertyAnimation::finished, this, [=]() {
        statusLabel->setText(status);
        statusOpacityAnimation->setStartValue(0.0);
        statusOpacityAnimation->setEndValue(1.0);
        statusOpacityAnimation->start();
        disconnect(statusOpacityAnimation, &QPropertyAnimation::finished, this, nullptr);
    });
    
    statusOpacityAnimation->start();
    QApplication::processEvents();
}

void LoadingView::updateProgress(int value)
{
    progressAnimation->stop();
    progressAnimation->setStartValue(progressBar->value());
    progressAnimation->setEndValue(value);
    progressAnimation->start();
    QApplication::processEvents();
}

void LoadingView::markLoadingComplete()
{
    QRect currentGeometry = appNameLabel->geometry();
    appNameAnimation->setStartValue(currentGeometry);
    
    QRect endGeometry = currentGeometry;
    endGeometry.translate(0, -10);
    
    appNameAnimation->setEndValue(endGeometry);
    appNameAnimation->start();
    
    updateStatus("Loading complete!");
    QTimer::singleShot(1000, this, &LoadingView::loadingFinished);
}
