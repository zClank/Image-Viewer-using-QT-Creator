#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QLabel>
#include <QScrollArea>
#include <QIntValidator>
#include <QPainter>
#include <QMessageBox>

#include <cassert>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionAbrir_Imagem, SIGNAL(triggered()), this, SLOT(selecionarImagem()));
    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(fecharImagem(int)));
    connect(ui->botaoZoomIn, SIGNAL(clicked()), this, SLOT(zoomInImagemAtual()));
    connect(ui->botaoZoomOut, SIGNAL(clicked()), this, SLOT(zoomOutImagemAtual()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(mudarImagem(int)));
    connect(ui->botaoMaisBrilho, SIGNAL(clicked()), this, SLOT(aumentarBrilho()));
    connect(ui->botaoMenosBrilho, SIGNAL(clicked()), this, SLOT(diminuirBrilho()));
    connect(ui->textoNivelDeBrilho, SIGNAL(editingFinished()), this, SLOT(atualizarDadosBrilho()));
    connect(ui->rotacionarAntiHorario, SIGNAL(clicked()), this, SLOT(rotacionarAntiHorario()));
    connect(ui->rotacionarHorario, SIGNAL(clicked()), this, SLOT(rotacionarHorario()));
    ui->botaoZoomIn->setEnabled(false);
    ui->botaoZoomOut->setEnabled(false);
    ui->textoNivelDeBrilho->setValidator(new QIntValidator(0, 100, this));
    ui->textoNivelDeBrilho->setEnabled(false);
    ui->botaoMaisBrilho->setEnabled(false);
    ui->botaoMenosBrilho->setEnabled(false);
    ui->rotacionarHorario->setEnabled(false);
    ui->rotacionarAntiHorario->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::selecionarImagem()
{
    const QString nomeDaImagem = QFileDialog::getOpenFileName(this,
                  tr("Abrir Imagem"), QString(), tr("Arquivos de Imagem (*.jpg)"));
    QPixmap pixmapImagem(nomeDaImagem);
    QLabel* imagem = new QLabel();
    imagem->setScaledContents(true);
    imagem->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored);
    QScrollArea* areaImagem = new QScrollArea();
    areaImagem->setWidget(imagem);
    //Calcula os fatores para poder aproximar a imagem do tamanho da janela.
    const double fatorLargura = static_cast<double>(ui->tabWidget->width()) / static_cast<double>(pixmapImagem.width());
    const double fatorAltura = static_cast<double>(ui->tabWidget->height()) / static_cast<double>(pixmapImagem.height());
    const double fatorMinimo = std::min(fatorLargura, fatorAltura);
    QPixmap pixmapAjustado = pixmapImagem.scaled(pixmapImagem.size() * fatorMinimo);
    atributosImagens.push_back(DadosImagem{});
    imagem->setPixmap(pixmapAjustado);
    imagem->adjustSize();
    const int indiceNovaAba = ui->tabWidget->addTab(areaImagem, nomeDaImagem);
    atributosImagens[indiceNovaAba].imagemOriginal = pixmapAjustado;
    ui->tabWidget->setCurrentIndex(indiceNovaAba);
}

void MainWindow::mudarImagem(int index)
{
    atualizarControles();
}

void MainWindow::fecharImagem(int index)
{
    QWidget* areaImagem = ui->tabWidget->widget(index);
    atributosImagens.erase(atributosImagens.begin() + index);
    ui->tabWidget->removeTab(index);
    delete areaImagem;
}

bool MainWindow::temImagem() const {
    return ui->tabWidget->count() != 0;
}

void MainWindow::zoomInImagemAtual()
{
    if(temImagem()) {
        //Fator de 1.2 escolhido devido ao fato de 1.2^5 =~ 2.5, para um fator de zoom maximo de cerca de 250%.
        const double passoEscala = 1.2;
        escalarImagem(passoEscala);
    }
}

void MainWindow::zoomOutImagemAtual()
{
    if(temImagem()) {
        //Passo escala in vezes passo escala out dar 1 para zoom in e zoom out se zerarem.
        const double passoEscala = 1.0 / 1.2;
        escalarImagem(passoEscala);
    }
}

void MainWindow::escalarImagem(double fator) {
    const int indexAtual = ui->tabWidget->currentIndex();
    atributosImagens[indexAtual].fatorEscala *= fator;
    ajustarImagemAtual();
    ajustarScrollArea(fator);
    atualizarControlesZoom();
}

void MainWindow::ajustarImagemAtual()
{
    QScrollArea* scrollArea = dynamic_cast<QScrollArea*>(ui->tabWidget->currentWidget());
    assert(scrollArea != nullptr);
    QLabel* imagem = dynamic_cast<QLabel*>(scrollArea->widget());
    assert(imagem != nullptr);
    int index = ui->tabWidget->currentIndex();
    //Nao eh necessario chamar esse metodo ao recarregar uma imagem, devido ao fato do label em si ja ter sido alterado.
    imagem->resize(atributosImagens[index].fatorEscala * imagem->pixmap().size());
}

void MainWindow::ajustarScrollArea(double fator)
{
    QScrollArea* scrollArea = dynamic_cast<QScrollArea*>(ui->tabWidget->currentWidget());
    assert(scrollArea != nullptr);
    QScrollBar* scrollBarHorizontal = scrollArea->horizontalScrollBar();
    QScrollBar* scrollBarVertical = scrollArea->verticalScrollBar();
    ajustarScrollBar(scrollBarHorizontal, fator);
    ajustarScrollBar(scrollBarVertical, fator);
}

void MainWindow::ajustarScrollBar(QScrollBar* scrollBar, double fator)
{
    scrollBar->setValue(fator * scrollBar->value() + (fator - 1) * scrollBar->pageStep() / 2.0);
}

void MainWindow::aumentarBrilho()
{
    const int index = ui->tabWidget->currentIndex();
    ++atributosImagens[index].brilho;
    ui->textoNivelDeBrilho->setText(QString::number(atributosImagens[index].brilho));
    aplicarTransformacoesImagem();
}

void MainWindow::diminuirBrilho()
{
    const int index = ui->tabWidget->currentIndex();
    --atributosImagens[index].brilho;
    aplicarTransformacoesImagem();
}

void MainWindow::atualizarDadosBrilho()
{
    if(temImagem()) {
        const int alpha = ui->textoNivelDeBrilho->text().toInt();
        const int index = ui->tabWidget->currentIndex();
        atributosImagens[index].brilho = alpha;
        aplicarTransformacoesImagem();
    }
}

void MainWindow::aplicarTransformacoesImagem()
{
    QScrollArea* scrollArea = dynamic_cast<QScrollArea*>(ui->tabWidget->currentWidget());
    assert(scrollArea != nullptr);
    QLabel* imagem = dynamic_cast<QLabel*>(scrollArea->widget());
    assert(imagem != nullptr);
    const int index = ui->tabWidget->currentIndex();
    const int alpha = atributosImagens[index].brilho;
    const int angulo = atributosImagens[index].rotacao;
    QPixmap output(imagem->pixmap().size());
    output.fill(Qt::transparent);
    QPainter painter(&output);
    //Centraliza a imagem para rotacionar ao redor do proprio eixo
    painter.translate(imagem->pixmap().width() / 2.0, imagem->pixmap().height() / 2.0);
    painter.rotate(angulo);
    //Volta a imagem para a posicao original
    painter.translate(-(imagem->pixmap().width() / 2.0), -(imagem->pixmap().height() / 2.0));
    //Aplica o brilho
    painter.setOpacity(0.01 * static_cast<double>(alpha));
    //Deve utilizar a imagem original como source. Se utilizar o pixmap do label, a imagem perde informacoes
    painter.drawPixmap(0, 0, atributosImagens[index].imagemOriginal);
    imagem->setPixmap(output);
    ajustarImagemAtual();
    atualizarControlesBrilho();
}

void MainWindow::atualizarControles()
{
    atualizarControlesZoom();
    atualizarControlesBrilho();
    atualizarControlesRotacao();
}

void MainWindow::atualizarControlesZoom()
{
    if(!temImagem()) {
        ui->botaoZoomIn->setEnabled(false);
        ui->botaoZoomOut->setEnabled(false);
    }
    else {
        const int index = ui->tabWidget->currentIndex();
        const double escalaAtual = atributosImagens[index].fatorEscala;
        ui->botaoZoomIn->setEnabled(escalaAtual < escalaMaxima);
        ui->botaoZoomOut->setEnabled(escalaAtual > escalaMinima);
    }
}

void MainWindow::atualizarControlesBrilho()
{
    if(temImagem()) {
        ui->textoNivelDeBrilho->setEnabled(true);
        const int index = ui->tabWidget->currentIndex();
        const int nivelBrilho = atributosImagens[index].brilho;
        ui->textoNivelDeBrilho->setText(QString::number(nivelBrilho));
        ui->botaoMaisBrilho->setEnabled(nivelBrilho < 100);
        ui->botaoMenosBrilho->setEnabled(nivelBrilho > 0);
    }
    else {
        ui->textoNivelDeBrilho->setEnabled(false);
        ui->textoNivelDeBrilho->setText(QString::number(0));
        ui->botaoMaisBrilho->setEnabled(false);
        ui->botaoMenosBrilho->setEnabled(false);
    }
}

void MainWindow::atualizarControlesRotacao()
{
    const bool deveHabilitar = temImagem();
    ui->rotacionarAntiHorario->setEnabled(deveHabilitar);
    ui->rotacionarHorario->setEnabled(deveHabilitar);
}

void MainWindow::rotacionarHorario()
{
    const int index = ui->tabWidget->currentIndex();
    atributosImagens[index].rotacao = (atributosImagens[index].rotacao + 90) % 360;
    aplicarTransformacoesImagem();
}

void MainWindow::rotacionarAntiHorario()
{
    const int index = ui->tabWidget->currentIndex();
    atributosImagens[index].rotacao = (atributosImagens[index].rotacao - 90) % 360;
    aplicarTransformacoesImagem();
}

