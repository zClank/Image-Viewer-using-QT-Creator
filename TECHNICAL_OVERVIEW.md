# Visão Técnica do Projeto — Pixeon Image Viewer

> Documento de preparação para bate-papo técnico com a equipe Pixeon.  
> Cobre: responsabilidade de cada arquivo, funções relevantes e glossário de Qt.

---

## 1. Estrutura Geral do Projeto

```
PixeonImageViewer/
├── main.cpp / main.h           → Ponto de entrada da aplicação
├── MainWindow.cpp / .h         → Janela principal: menus, docks, layout
├── ImageViewer.cpp / .h        → Área de visualização (QGraphicsView)
├── CanvasImageItem.cpp / .h    → Item de imagem no canvas (QGraphicsPixmapItem)
├── ImageProcessor.cpp / .h     → Processamento matemático de pixels
└── CMakeLists.txt              → Configuração de build com CMake + Qt
```

---

## 2. Resumo de Cada Arquivo

### `main.cpp` + `main.h`
**O que faz:** Ponto de entrada do programa.

- Cria o objeto `QApplication`, que é obrigatório em qualquer aplicação Qt e gerencia o **event loop** (laço de eventos de UI).
- Define o nome da aplicação e da organização via `QCoreApplication::setApplicationName` e `QCoreApplication::setOrganizationName` — isso é usado pelo Qt para nomear arquivos de configuração automáticos do sistema operacional.
- Instancia o `MainWindow` e chama `window.show()` para exibí-lo.
- Chama `app.exec()`, que bloqueia a execução e fica aguardando eventos do usuário (cliques, teclas, etc.) até a janela ser fechada.

---

### `MainWindow.cpp` + `MainWindow.h`
**O que faz:** Define a janela principal da aplicação e toda a estrutura visual de alto nível.

**Responsabilidades:**
- Cria os menus (`File`, `Edit`, `View`) com suas ações.
- Cria dois **DockWidgets** (painéis encaixáveis laterais):
  - **Esquerdo:** lista de imagens abertas (`QListWidget`).
  - **Direito:** controles de ferramentas (`QComboBox`) e sliders de brilho/contraste.
- Gerencia o modo Dark/Light ao manipular a paleta de cores global (`QPalette`).
- Conecta os sinais (eventos) da UI às funções de ação usando o mecanismo **Signal & Slot** do Qt.
- Serve como **orquestrador**: recebe eventos da UI e delega a lógica para `ImageViewer`.

**Funções relevantes:**

| Função | O que faz |
|---|---|
| `createActions()` | Cria objetos `QAction` que representam cada comando do menu. |
| `createMenus()` | Monta a barra de menus usando os `QAction` criados anteriormente. |
| `createDockWindows()` | Cria os painéis laterais com todos os controles e sliders. |
| `toggleDarkMode()` | Altera a paleta de cores da aplicação inteira para simular Dark Mode usando `QPalette`. |
| `imageSelectionChanged()` | Responde quando o usuário clica em outra imagem na lista — carrega a nova imagem e reseta os sliders. |
| `brightnessSliderChanged()` / `contrastSliderChanged()` | Propagam a mudança do slider para o `ImageViewer`, que por sua vez repassa ao item ativo. |

---

### `ImageViewer.cpp` + `ImageViewer.h`
**O que faz:** A área central de visualização da imagem, que herda de `QGraphicsView`.

**Responsabilidades:**
- Gerencia uma `QGraphicsScene` — o "mundo 2D" onde os itens de imagem vivem.
- Implementa o sistema de **modos de interação** (enum `Mode`): Pan/Zoom, Ajuste de Intensidade, Recorte, Desenho.
- Intercepta eventos de mouse (`mousePressEvent`, `mouseMoveEvent`, `mouseReleaseEvent`) e teclado (`keyPressEvent`) para executar as ferramentas.
- Controla o zoom via roda do mouse (`wheelEvent`).
- Gerencia múltiplas imagens no **Canvas Mode** (modo lado-a-lado).
- Emite sinais (`brightnessChanged`, `contrastChanged`) para manter os sliders do painel lateral sincronizados.

**Funções relevantes:**

| Função | O que faz |
|---|---|
| `loadImage()` | Cria um `CanvasImageItem` e o adiciona à cena. No modo normal, apaga a imagem anterior; no Canvas Mode, empilha os itens com offset. |
| `setMode()` | Troca o modo de interação. No modo Pan, ativa `ScrollHandDrag`; nos outros modos desativa o drag nativo para não interferir com as ferramentas. |
| `finalizeCrop()` | Converte as coordenadas do `QRubberBand` (em pixels de tela) para coordenadas do item de imagem e executa o recorte. É necessário passar por duas transformações de coordenadas: viewport → cena → item. |
| `mouseMoveEvent()` | No modo `AdjustIntensity`, usa o delta do movimento do mouse (eixo X = brilho, eixo Y = contraste). No modo `Draw`, converte coordenadas de tela para o espaço do item usando `mapFromScene(mapToScene(...))`. |
| `getActiveItem()` | Retorna o `CanvasImageItem` atualmente selecionado. Se não houver seleção mas só existir um item, ele é retornado por padrão. |
| `undoLastAction()` | Chama `undo()` no item ativo e emite os sinais para atualizar os sliders com os valores anteriores. |
| `keyPressEvent()` | Rotaciona a imagem 90° para esquerda (tecla Q) ou direita (tecla E) usando `setRotation()`, com a origem da transformação centralizada. |

---

### `CanvasImageItem.cpp` + `CanvasImageItem.h`
**O que faz:** Representa cada imagem como um item independente e interativo na cena gráfica. Herda de `QGraphicsPixmapItem`.

**Responsabilidades:**
- Carrega a imagem do disco e mantém duas versões: `originalImage` (intocada) e `processedImage` (com brilho/contraste aplicados).
- Gerencia uma lista de linhas desenhadas pelo usuário (`drawnLines`).
- Implementa um sistema de **undo** com uma pilha de estados (`undoStack`).
- Faz o `override` do método `paint()` para renderizar: (1) a imagem, (2) as linhas desenhadas, (3) a borda de seleção azul.

**Estrutura `State` (undo):**
```cpp
struct State {
    QImage originalImage;  // snapshot da imagem antes da modificação
    int brightness;
    int contrast;
    QList<QLine> drawnLines;
};
```

**Funções relevantes:**

| Função | O que faz |
|---|---|
| `pushUndoState()` | Salva um snapshot do estado atual na pilha antes de qualquer modificação destrutiva. |
| `undo()` | Remove o topo da pilha e restaura o estado anterior, chamando `applyIntensity()` para redesenhar. |
| `applyIntensity()` | Chama `ImageProcessor` para recalcular a imagem processada e atualiza o pixmap exibido via `setPixmap()`. Chama `update()` para forçar o redesenho do item. |
| `crop()` | Usa `QImage::copy(rect)` para extrair uma sub-região da imagem original e descarta as linhas desenhadas (pois as coordenadas seriam inválidas). |
| `getFinalImage()` | Compõe a imagem final combinando o `processedImage` com as linhas desenhadas usando `QPainter`, para exportação. |
| `paint()` | Override do Qt chamado automaticamente no repaint. Desenha a imagem base, as anotações em vermelho e a borda de seleção azul se o item estiver selecionado. |

---

### `ImageProcessor.cpp` + `ImageProcessor.h`
**O que faz:** Módulo utilitário puro para processamento matemático de pixels. Não tem estado nem herança Qt.

**Função única:**

```cpp
static QImage applyBrightnessAndContrast(const QImage &image, int brightness, int contrast);
```

**Como funciona (passo a passo):**

1. **Converte** o formato da imagem para `QImage::Format_RGB32` — garante que cada pixel ocupa exatamente 4 bytes (R, G, B, + padding), permitindo acesso direto e eficiente via ponteiro.

2. **Normaliza** os valores de brilho e contraste:
   - Brilho: escala de `-100..100` para `-255..+255` (deslocamento direto nos canais RGB).
   - Contraste: calcula um fator multiplicador (`cFactor`):
     - Contraste positivo → amplia a distância dos pixels em relação ao meio-tom (128).
     - Contraste negativo → comprime essa distância.

3. **Itera** linha a linha usando `QImage::scanLine(y)`, que retorna um ponteiro direto para a memória da linha (mais rápido que `pixel(x, y)` e `setPixel(x, y)`).

4. Para cada pixel: aplica a fórmula de contraste `(cor - 128) * fator + 128`, adiciona o deslocamento de brilho, e reclampa o resultado no range `[0, 255]` com `std::clamp`.

5. Escreve o pixel modificado de volta com `qRgba(r, g, b, alpha)`, preservando o canal alfa.

---

## 3. Funções e Classes Exclusivas do Qt Utilizadas

### Arquitetura e Ciclo de Vida

| Elemento Qt | Arquivo | O que é / Para que serve |
|---|---|---|
| `QApplication` | `main.cpp` | Objeto raiz de qualquer app Qt com UI. Gerencia o event loop, configurações globais e recursos de estilo. Obrigatório antes de qualquer widget. |
| `QCoreApplication::setApplicationName()` | `main.cpp` | Define o nome da aplicação para fins de metadados e armazenamento de configurações (ex: QSettings). |
| `Q_OBJECT` | Todos os headers | Macro obrigatória em classes que usam sinais, slots ou `tr()`. Gera código extra pelo MOC (Meta-Object Compiler) do Qt. |
| `app.exec()` | `main.cpp` | Inicia o laço de eventos Qt. Bloqueia a execução até a janela ser fechada. |

---

### Janela e Widgets

| Elemento Qt | Arquivo | O que é / Para que serve |
|---|---|---|
| `QMainWindow` | `MainWindow.h` | Classe base para janelas principais com suporte nativo a menus, toolbars e docks. |
| `QDockWidget` | `MainWindow.cpp` | Painel encaixável ("dock") que pode ser movido e redimensionado pelo usuário. |
| `QListWidget` | `MainWindow.cpp` | Lista de itens clicáveis. Usado para exibir as imagens carregadas. Cada item é um `QListWidgetItem`. |
| `QSlider` | `MainWindow.cpp` | Controle deslizante. `setRange()` define os limites; `setValue()` define o valor atual. |
| `QComboBox` | `MainWindow.cpp` | Dropdown de seleção. Usado para escolher o modo da ferramenta. |
| `QGroupBox` | `MainWindow.cpp` | Caixa com borda e título usada para agrupar controles visualmente. |
| `QLabel` | `MainWindow.cpp` | Texto estático exibido na interface. |
| `QFormLayout` | `MainWindow.cpp` | Layout em duas colunas (label + widget). Ideal para formulários. |
| `QVBoxLayout` | `MainWindow.cpp` | Organiza widgets verticalmente. |
| `setCentralWidget()` | `MainWindow.cpp` | Define qual widget ocupa a área central da `QMainWindow`. |
| `resize()` | `MainWindow.cpp` | Define o tamanho inicial da janela em pixels. |

---

### Menus e Ações

| Elemento Qt | Arquivo | O que é / Para que serve |
|---|---|---|
| `QAction` | `MainWindow.cpp` | Representa uma ação (comando) que pode aparecer em menus, toolbars ou via atalho de teclado. |
| `QAction::setCheckable()` | `MainWindow.cpp` | Torna a ação um "toggle" — ela pode estar marcada ou desmarcada (ex: Canvas Mode). |
| `QKeySequence::Undo` | `MainWindow.cpp` | Constante Qt para o atalho padrão de desfazer (`Ctrl+Z` no Windows/Linux, `Cmd+Z` no Mac). |
| `menuBar()->addMenu()` | `MainWindow.cpp` | Acessa a barra de menus nativa da `QMainWindow` e adiciona um menu. |
| `tr()` | Vários | Marca strings para internacionalização (i18n). Permite que o Qt Linguist traduza os textos. |

---

### Sinais e Slots

| Elemento Qt | Arquivo | O que é / Para que serve |
|---|---|---|
| `connect()` | Vários | Conecta um sinal (evento emitido) a um slot (função receptora). Pode usar ponteiro de função (Qt5) ou lambdas. |
| `emit` | `ImageViewer.cpp` | Palavra-chave do Qt para disparar um sinal definido na classe. |
| `blockSignals(true/false)` | `MainWindow.cpp` | Bloqueia temporariamente os sinais de um widget para evitar loops de feedback ao atualizar seu valor programaticamente. |
| `signals:` | `ImageViewer.h` | Seção do header onde são declarados os sinais da classe. São funções sem implementação — o MOC gera o código. |
| `private slots:` | `MainWindow.h` | Seção onde são declarados os slots (funções que podem ser conectadas a sinais). |
| `QOverload<int>::of(...)` | `MainWindow.cpp` | Resolve ambiguidade de sobrecarga em `connect()`. Necessário porque `QComboBox::currentIndexChanged` tem duas versões (uma com `int`, outra com `QString`). |

---

### Gráficos (QGraphicsView Framework)

| Elemento Qt | Arquivo | O que é / Para que serve |
|---|---|---|
| `QGraphicsView` | `ImageViewer.h` | Widget que exibe uma `QGraphicsScene`. Gerencia zoom, pan, transformações de coordenadas e renderização. |
| `QGraphicsScene` | `ImageViewer.cpp` | O "mundo 2D" onde vivem os itens gráficos. Não é exibido diretamente — precisa de uma `QGraphicsView`. |
| `QGraphicsPixmapItem` | `CanvasImageItem.h` | Item gráfico especializado em exibir um `QPixmap`. Herda de `QGraphicsItem` e fornece seleção e movimentação. |
| `QGraphicsTextItem` | `ImageViewer.cpp` | Item de texto renderizado dentro da cena (usado para o placeholder). |
| `setScene()` | `ImageViewer.cpp` | Associa a `QGraphicsScene` ao `QGraphicsView`. |
| `fitInView()` | `ImageViewer.cpp` | Ajusta o zoom da view para que o item especificado caiba completamente na tela. `Qt::KeepAspectRatio` mantém as proporções. |
| `setSceneRect()` | `ImageViewer.cpp` | Define os limites do "mundo" da cena. Afeta onde as barras de rolagem aparecem. |
| `setDragMode()` | `ImageViewer.cpp` | `ScrollHandDrag` = cursor de mão para arrastar a cena; `NoDrag` = sem drag nativo (para ferramentas customizadas). |
| `setRenderHint()` | `ImageViewer.cpp` | Ativa suavizações: `Antialiasing` suaviza bordas; `SmoothPixmapTransform` usa interpolação bilinear no zoom. |
| `setFlags()` | `CanvasImageItem.cpp` | Define as capacidades do item: `ItemIsSelectable` (clicável) e `ItemIsMovable` (arrastável). |
| `mapToScene()` | `ImageViewer.cpp` | Converte coordenadas do **viewport** (pixels da janela) para coordenadas da **cena**. |
| `mapFromScene()` | `ImageViewer.cpp` | Converte coordenadas da **cena** para coordenadas **locais do item**. |
| `boundingRect()` | `ImageViewer.cpp`, `CanvasImageItem.cpp` | Retorna o retângulo que envolve o item no seu espaço de coordenadas locais. |
| `scene->addItem()` | `ImageViewer.cpp` | Adiciona um item à cena para que seja exibido. |
| `scene->removeItem()` | `ImageViewer.cpp` | Remove um item da cena (sem deletar da memória). |
| `scene->clearSelection()` | `ImageViewer.cpp` | Deseleciona todos os itens da cena. |
| `scene->selectedItems()` | `ImageViewer.cpp` | Retorna lista de itens selecionados. |
| `scene->itemsBoundingRect()` | `ImageViewer.cpp` | Retorna o menor retângulo que contém todos os itens da cena. |
| `QGraphicsScene::selectionChanged` | `ImageViewer.cpp` | Sinal emitido quando a seleção de itens muda. Conectado para sincronizar os sliders. |
| `update()` | `CanvasImageItem.cpp` | Solicita ao Qt que redesenhe o item (chama `paint()` na próxima oportunidade). |
| `isSelected()` | `CanvasImageItem.cpp` | Verifica se o item está selecionado na cena. |

---

### Imagem e Pixmap

| Elemento Qt | Arquivo | O que é / Para que serve |
|---|---|---|
| `QImage` | Vários | Representação de imagem em memória com acesso direto aos pixels. Ideal para processamento. |
| `QPixmap` | Vários | Representação otimizada para exibição na tela. Armazenada na memória da GPU quando possível. |
| `QPixmap::fromImage()` | `CanvasImageItem.cpp` | Converte um `QImage` (processamento) para `QPixmap` (exibição). |
| `setPixmap()` | `CanvasImageItem.cpp` | Atualiza o pixmap exibido pelo `QGraphicsPixmapItem`. |
| `QImage::load()` | `CanvasImageItem.cpp` | Carrega uma imagem do disco. Suporta PNG, JPEG, BMP, etc. Retorna `true` se bem-sucedido. |
| `QImage::copy(rect)` | `CanvasImageItem.cpp` | Retorna uma cópia recortada da imagem na região especificada. |
| `QImage::convertToFormat()` | `ImageProcessor.cpp` | Converte a imagem para um formato específico de pixel. `Format_RGB32` = 4 bytes por pixel, sem alfa real. |
| `QImage::scanLine(y)` | `ImageProcessor.cpp` | Retorna ponteiro para o início da linha `y` na memória da imagem. Acesso direto e eficiente. |
| `QImage::isNull()` | `ImageProcessor.cpp` | Verifica se a imagem está vazia/inválida. |

---

### Funções Matemáticas de Pixel

| Elemento Qt | Arquivo | O que é / Para que serve |
|---|---|---|
| `qRed()`, `qGreen()`, `qBlue()`, `qAlpha()` | `ImageProcessor.cpp` | Extrai um canal de cor de um valor `QRgb` (que é um `unsigned int`). |
| `qRgba(r, g, b, a)` | `ImageProcessor.cpp` | Empacota quatro canais de cor em um único `QRgb`. |
| `QRgb` | `ImageProcessor.cpp` | Typedef Qt para `unsigned int` representando um pixel ARGB. |
| `qBound(min, val, max)` | `ImageViewer.cpp` | Equivalente a `clamp` — limita um valor entre mínimo e máximo. |

---

### Desenho (QPainter)

| Elemento Qt | Arquivo | O que é / Para que serve |
|---|---|---|
| `QPainter` | `CanvasImageItem.cpp` | API de desenho 2D do Qt. Usado tanto no `paint()` (renderização em tela) quanto no `getFinalImage()` (exportação). |
| `QPen` | `CanvasImageItem.cpp` | Define estilo da linha: cor, espessura, tipo (sólida), cap (RoundCap) e join (RoundJoin). |
| `painter->save()` / `restore()` | `CanvasImageItem.cpp` | Salva e restaura o estado do `QPainter` (cor, pen, transform) para não afetar o código seguinte. |
| `painter->setPen()` | `CanvasImageItem.cpp` | Define o lápis de desenho ativo no `QPainter`. |
| `painter->setBrush()` | `CanvasImageItem.cpp` | Define o pincel de preenchimento. `Qt::NoBrush` = sem preenchimento. |
| `painter->drawLine()` | `CanvasImageItem.cpp` | Desenha uma linha entre dois pontos. |
| `painter->drawRect()` | `CanvasImageItem.cpp` | Desenha um retângulo (sem preenchimento, com o pen atual). |
| `QGraphicsPixmapItem::paint()` | `CanvasImageItem.cpp` | Método base que desenha o pixmap. Chamado explicitamente dentro do `paint()` override para manter o comportamento padrão antes de adicionar as anotações. |

---

### Eventos de Mouse e Teclado

| Elemento Qt | Arquivo | O que é / Para que serve |
|---|---|---|
| `QWheelEvent` | `ImageViewer.cpp` | Evento de roda do mouse. `angleDelta().y()` retorna o quanto a roda girou. |
| `QMouseEvent` | `ImageViewer.cpp` | Evento de clique/movimento do mouse. `event->pos()` retorna coordenadas no viewport. |
| `QKeyEvent` | `ImageViewer.cpp` | Evento de teclado. `event->key()` retorna a tecla pressionada; `event->modifiers()` verifica Ctrl, Shift, etc. |
| `QGraphicsView::mousePressEvent()` | `ImageViewer.cpp` | Versão base do evento. Chamada explicitamente no final dos overrides para garantir que o Qt processe a seleção de itens normalmente. |
| `event->modifiers()` | `ImageViewer.cpp` | Retorna as teclas modificadoras ativas (Ctrl, Shift, Alt). Comparado com `Qt::ControlModifier`. |
| `scale(sx, sy)` | `ImageViewer.cpp` | Aplica uma transformação de escala à viewport. Usado para implementar o zoom. |
| `setTransformOriginPoint()` | `ImageViewer.cpp` | Define o ponto de pivô para rotação/escala de um item gráfico. Centralizado em `boundingRect().center()` para rotação no próprio eixo. |
| `setRotation()` / `rotation()` | `ImageViewer.cpp` | Define/lê o ângulo de rotação de um `QGraphicsItem` em graus. |

---

### Outros Utilitários

| Elemento Qt | Arquivo | O que é / Para que serve |
|---|---|---|
| `QRubberBand` | `ImageViewer.h/cpp` | Widget de seleção visual (retângulo pontilhado). Exibido sobre o viewport para indicar a área de recorte. |
| `QRubberBand::setGeometry()` | `ImageViewer.cpp` | Define a posição e tamanho do rubber band em coordenadas do viewport. `QRect(...).normalized()` garante que o retângulo sempre tenha largura/altura positivas independente da direção do arrasto. |
| `QFileDialog::getOpenFileName()` | `MainWindow.cpp` | Abre o diálogo nativo do SO para seleção de **um** arquivo. |
| `QFileDialog::getOpenFileNames()` | `MainWindow.cpp` | Abre o diálogo nativo para seleção de **múltiplos** arquivos. |
| `QFileDialog::getSaveFileName()` | `MainWindow.cpp` | Abre o diálogo nativo para salvar um arquivo. |
| `QFileInfo::fileName()` | `MainWindow.cpp` | Extrai apenas o nome do arquivo a partir de um caminho completo (ex: `"/path/to/img.png"` → `"img.png"`). |
| `QListWidgetItem::setData()` | `MainWindow.cpp` | Armazena dados extras em um item da lista. `Qt::UserRole` é a role padrão para dados customizados do desenvolvedor (aqui guarda o caminho completo do arquivo). |
| `QPalette` | `MainWindow.cpp` | Define o esquema de cores da interface. Modificado para implementar o Dark Mode via `setColor(role, cor)`. |
| `qApp->setStyle("Fusion")` | `MainWindow.cpp` | Aplica o estilo visual "Fusion" — necessário para que o Dark Mode customizado funcione corretamente em todas as plataformas. |
| `QColor(r, g, b)` / `QColor(r, g, b, a)` | `MainWindow.cpp`, `CanvasImageItem.cpp` | Representa uma cor. Pode ser criada com valores RGB, RGBA ou nomes (ex: `Qt::white`, `Qt::red`). |
| `dynamic_cast<>()` | `ImageViewer.cpp` | Cast seguro em tempo de execução. Usado para converter `QGraphicsItem*` (tipo base) para `CanvasImageItem*` (tipo derivado). Retorna `nullptr` se a conversão falhar. |
| `QList<T>` | Vários | Container genérico do Qt similar ao `std::vector`. Usado para listas de itens, linhas e estados de undo. |
| `QStringList` | `MainWindow.cpp` | Especialização de `QList<QString>`. Retornada por `getOpenFileNames()`. |
| `Qt::KeepAspectRatio` | `ImageViewer.cpp` | Enum usado em `fitInView()` para manter a proporção da imagem ao ajustar ao tamanho da tela. |

---

## 4. Conceitos Importantes para a Discussão

### Sistema de Coordenadas do QGraphicsView
O framework usa **três espaços de coordenadas** distintos:
1. **Viewport** — coordenadas em pixels da janela do widget (`event->pos()`).
2. **Scene** — coordenadas do "mundo" 2D da cena (`mapToScene()`).
3. **Item** — coordenadas locais de cada item gráfico (`mapFromScene()`).

A função `finalizeCrop()` e o modo `Draw` precisam converter viewport → scene → item para que o recorte e o desenho sejam aplicados corretamente independente de zoom, pan ou rotação.

### Signal & Slot — O Mecanismo de Eventos do Qt
Ao invés de callbacks diretos, Qt usa **sinais** (eventos emitidos) e **slots** (funções receptoras) desacoplados. O `connect()` cria a ligação. Isso permite que `MainWindow` escute mudanças em `ImageViewer` sem que um precise conhecer os detalhes do outro.

O `blockSignals(true)` é uma técnica para evitar **loops de feedback**: quando o código atualiza um slider programaticamente, não queremos que isso dispare novamente a função de "usuário mudou o slider".

### QImage vs QPixmap
- `QImage` vive na RAM da CPU e permite acesso direto a pixels — ideal para `ImageProcessor`.
- `QPixmap` é otimizado para exibição (pode viver na GPU) — ideal para `QGraphicsPixmapItem`.
- O fluxo sempre é: processar com `QImage` → converter com `QPixmap::fromImage()` → exibir com `setPixmap()`.

### Sistema de Undo Simples (Stack de Estados)
Sem usar `QUndoStack` do Qt, o projeto implementa manualmente uma pilha de estados (`QList<State> undoStack`). Antes de qualquer ação destrutiva, `pushUndoState()` salva um snapshot completo; `undo()` usa `takeLast()` para remover e restaurar o estado anterior.

### Override do `paint()` em QGraphicsPixmapItem
O método `paint()` é chamado automaticamente pelo framework de renderização do Qt sempre que o item precisa ser redesenhado. O override em `CanvasImageItem` chama primeiro `QGraphicsPixmapItem::paint()` (para desenhar a imagem base) e depois adiciona as anotações e borda de seleção manualmente com `QPainter`. É importante usar `painter->save()` e `painter->restore()` para não "vazar" configurações de desenho entre as etapas.
