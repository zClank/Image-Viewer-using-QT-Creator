#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScrollBar>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    /**
     * Construtor padrao janela.
     *
     * @param parent Widget pai da janela. Este widget toma ownership deste objeto.
     */
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    /**
     * Metodo chamado apos clicar no botao de abrir imagem na interface. Seleciona uma imagem do disco para ser aberta no programa.
     */
    void selecionarImagem();

    /**
     * Metodo chamado ao trocar de aba no QTabWidget.
     *
     * @param index O novo indice do QTabWidget
     */
    void mudarImagem(int index);

    /**
     * Metodo chamado ao fechar uma imagem pela aba do QTabWidget.
     *
     * @param index o indice da imagem fechada
     */
    void fecharImagem(int index);

    /**
     * Metodo chamado ao clicar no botao de zoom in.
     */
    void zoomInImagemAtual();

    /**
     * Metodo chamado ao clicar no botao de zoom out.
     */
    void zoomOutImagemAtual();

    /**
     * Metodo chamado ao clicar no botao de aumentar o brilho da janela.
     */
    void aumentarBrilho();

    /**
     * Metodo chamado ao clicar no botao de diminuir o brilho da janela.
     */
    void diminuirBrilho();

    /**
     * Metodo chamado ao editar o QLineEdit que representa o valor numerico do brilho da janela.
     */
    void atualizarDadosBrilho();

    /**
     * Metodo chamado ao clicar no botao de rotacao <
     */
    void rotacionarHorario();

    /**
     * Metodo chamado ao clicar no botao de rotacao >
     */
    void rotacionarAntiHorario();

private:
    /**
     * Altera o valor de escapa da imagem atual
     *
     * @param fator O fator de escala a ser aplicado no fator atual.
     */
    void escalarImagem(double fator);

    /**
     * Aplica a escala da imagem atual na imagem.
     */
    void ajustarImagemAtual();

    /**
     * Ajusta os scrolls apos a alteracao de uma imagem
     *
     * @param fator O fator de escala aplicado
     */
    void ajustarScrollArea(double fator);

    /**
     * Ajusta um dos scroll bars da scroll area
     *
     * @param scrollBar Scroll Bar que representa ou o sentido horizontal ou vertical
     * @param fator O fator de escala.
     */
    void ajustarScrollBar(QScrollBar* scrollBar, double fator);

    /**
     * Verifica se existe alguma imagem na aplicacao
     *
     * @return true caso a aplicacao tenha alguma imagem
     */
    bool temImagem() const;

    /**
     * Aplica o brilho e a rotacao na imagem atual
     */
    void aplicarTransformacoesImagem();

    /**
     * Atualiza todos os controles
     */
    void atualizarControles();

    /**
     * Atualiza apenas os controles de zoom
     */
    void atualizarControlesZoom();

    /**
     * Atualiza apenas os controles de brilho
     */
    void atualizarControlesBrilho();

    /**
     * Atualiza apenas os controles de rotacao
     */
    void atualizarControlesRotacao();


    /**
     * Estrutura que guarda atributos referentes a cada imagem.
     * Cada imagem possui um brilho, uma escala atual, uma rotacao e tambem guarda o pixmap original para aplicar as transformacoes
     */
    struct DadosImagem
    {
        double fatorEscala = 1.0;
        int brilho = 100;
        int rotacao = 0;
        QPixmap imagemOriginal;
    };

    static constexpr double escalaMinima = 0.41;
    static constexpr double escalaMaxima = 2.45;

    Ui::MainWindow *ui;
    std::vector<DadosImagem> atributosImagens;
};
#endif // MAINWINDOW_H
