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

LoadingView::~LoadingView() {
    delete progressAnimation;
    progressAnimation = nullptr;
    delete statusOpacityAnimation;
    statusOpacityAnimation = nullptr;
    delete appNameAnimation;
    appNameAnimation = nullptr;
}

void LoadingView::setupUi() {
    createLayout();
    setupAppNameLabel();
    setupProgressBar();
    setupStatusLabel();
    
    setWindowTitle("Loading Elometry");
    setMinimumSize(500, 350);
}

void LoadingView::createLayout() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    mainLayout->setSpacing(30);
    mainLayout->setAlignment(Qt::AlignCenter);
    
    mainLayout->addStretch();
    mainLayout->addWidget(appNameLabel = new QLabel("Elometry", this));
    mainLayout->addWidget(progressBar = new QProgressBar(this));
    mainLayout->addWidget(statusLabel = new QLabel("Initializing", this));
    mainLayout->addStretch();
}

void LoadingView::setupAppNameLabel() {
    QFont headerFont("Segoe UI", 36);
    headerFont.setBold(true);
    appNameLabel->setFont(headerFont);
    appNameLabel->setStyleSheet("color: #0c7bb3;");
    appNameLabel->setAlignment(Qt::AlignCenter);
}

void LoadingView::setupProgressBar() {
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setMinimumWidth(400);
    progressBar->setMinimumHeight(10);
    progressBar->setMaximumHeight(10);
    progressBar->setTextVisible(false);
}

void LoadingView::setupStatusLabel() {
    statusOpacityEffect = new QGraphicsOpacityEffect(this);
    statusOpacityEffect->setOpacity(1.0);
    
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setGraphicsEffect(statusOpacityEffect);
    QFont statusFont("Segoe UI", 12);
    statusLabel->setFont(statusFont);
    statusLabel->setStyleSheet("color: #a0a0a0;");
}

void LoadingView::setupAnimations() {
    setupProgressAnimation();
    setupStatusAnimation();
    setupAppNameAnimation();
}

void LoadingView::setupProgressAnimation() {
    progressAnimation = new QPropertyAnimation(progressBar, "value", this);
    progressAnimation->setDuration(300);
    progressAnimation->setEasingCurve(QEasingCurve::OutQuad);
}

void LoadingView::setupStatusAnimation() {
    statusOpacityAnimation = new QPropertyAnimation(statusOpacityEffect, "opacity", this);
    statusOpacityAnimation->setDuration(250);
    statusOpacityAnimation->setEasingCurve(QEasingCurve::InOutQuad);
}

void LoadingView::setupAppNameAnimation() {
    appNameAnimation = new QPropertyAnimation(appNameLabel, "geometry", this);
    appNameAnimation->setDuration(800);
    appNameAnimation->setEasingCurve(QEasingCurve::OutBack);
}

void LoadingView::updateStatus(const QString& status) {
    statusOpacityAnimation->setStartValue(1.0);
    statusOpacityAnimation->setEndValue(0.0);
    
    disconnect(statusOpacityAnimation, &QPropertyAnimation::finished, this, nullptr);
    
    connect(statusOpacityAnimation, &QPropertyAnimation::finished, this, [this, status]() {
        statusLabel->setText(status);
        statusOpacityAnimation->setStartValue(0.0);
        statusOpacityAnimation->setEndValue(1.0);
        statusOpacityAnimation->start();
        disconnect(statusOpacityAnimation, &QPropertyAnimation::finished, this, nullptr);
    });
    
    statusOpacityAnimation->start();
    QApplication::processEvents();
}

void LoadingView::updateProgress(int value) {
    progressAnimation->stop();
    progressAnimation->setStartValue(progressBar->value());
    progressAnimation->setEndValue(value);
    progressAnimation->start();
    QApplication::processEvents();
}

void LoadingView::markLoadingComplete() {
    QRect currentGeometry = appNameLabel->geometry();
    appNameAnimation->setStartValue(currentGeometry);
    
    QRect endGeometry = currentGeometry;
    endGeometry.translate(0, -10);
    
    appNameAnimation->setEndValue(endGeometry);
    appNameAnimation->start();
    
    updateStatus("Loading complete!");
    
    QTimer::singleShot(1000, this, &LoadingView::loadingFinished);
}
