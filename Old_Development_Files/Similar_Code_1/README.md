# Projeto para exibição de imagem para realização do desafio da Pixeon

Essa aplicação implementa as funcionalidades de abrir múltiplas imagens no formato .jpg, 
disponibilizando um simples controle pela interface gráfica para operações como zoom, brilho e rotação.
Também é possível mexer no scroll para navegar por diferentes partes da imagem.

## Metodologia 

Para o desenvolvimento da aplicação, foi utilizada a linguagem C++ com o framework Qt. Em um primeiro momento, 
foram implementadas apenas as funções de procurar uma imagem pelo menu principal e abrir essa imagem através de 
um QLabel. Depois, foram considerados diferentes métodos para a implementação da funcionalidade de abrir várias imagens.

### Múltiplas imagens

A primeira alternativa considerada foi a manutenção do QLabel, salvando os vários QPixmap de cada imagem e dando um
resize nesse QLabel toda vez que fosse realizada uma troca, de forma a manter o tamanho natural. Para a seleção 
do arquivo atual, foi considerado o uso de uma QListWidget, fazendo atualização manual e salvando todas as informações
necessárias para as atualizações em um std::vector\<DadosImagem\>, sendo DadosImagem uma estrutura de dados
que armazena as diversas informações para cada uma das imagens salvadas. Essa ideia foi logo descartada, sendo
substituída pela utilização de um QTabWidget em que cada tab é uma QLabel com as dimensões corretas para cada imagem
e um QPixmap que contém a imagem em si. Desta forma, a troca de aba do QTabWidget realiza automaticamente a troca
entre as diferentes imagens.

Também notou-se que, ao remover um item da QTabWidget, a ownership da aba removida não pertence mais ao objeto,
sendo necessário fazer uma chamada manual com delete, respondendo ao SIGNAL tabCloseRequested(int).

### Zoom

Para a implementação do zoom, foi encontrada certa dificuldade. Em um primeiro momento, notou-se a necessidade de 
armazenar os diferentes fatores de escala atual para cada uma das imagens, e também a necessidade de implementar
um controle para a funcionalidade de zoom. Para manter a escala atual de cada imagem, foi implementado um
std::vector\<double\> cujo tamanho deve ser o mesmo do QTabWidget, e onde cada um dos índices corresponde a
cada uma das abas em uma relação de 1 para 1. Além disso, para adicionar a funcionalidade de scroll após dar o zoom
e ajustar o tamanho do widget automaticamente, estudou-se que uma QScrollArea tem toda essa funcionalidade.
Desta forma, cada aba do QTabWidget foi alterada de uma QLabel para uma QScrollArea cujo widget é uma QLabel, e o tamanho da 
imagem base é ajustado com um fator para preservar suas proporções originais mas tentar aproximar com o tamanho da tela.
Foi encontrado uma dificuldade também em preservar as posições da aba após dar um zoom, mas um estudo na internet
também resolveu esse problema, com a fórmula 

```
scrollBar->setValue(fator * scrollBar->value() + (fator - 1) * scrollBar->pageStep() / 2.0);
```

A última parte da implementação do zoom então vem da necessidade de atualização dos controles. Como um zoom in 
e um zoom out devem se cancelar, fatorZoomIn/fatorZoomOut deve ser igual a 1. Para tanto, um fator de zoom in de
1.2 foi escolhido, devido a aplicação de 5x esse fator chegar bem próximo a 2.5. Caso o fator de zoom então seja maior
que um máximo permitido de 2.45, o controle de zoom in é desabilitado. O zoom out funciona da mesma forma, mantendo a 
simetria com um valor mínimo de 1.2^-5, para balancear com o zoom in de 1.2^5. Também foi notado a necessidade de 
sincronizar o vetor de fatores com a QTabWidget, sendo removido o elemento na posição dentro do método 
tabCloseRequested(int). Os controles então devem ser atualizados tanto na mudança de aba, quanto na remoção de aba,
pois não se deve permitir zoom caso não tenha imagem.

### Brilho

Para a implementação do brilho, foi decidido a utilização de controles por botões e QLineEdit, utilizando valores inteiros
entre 0 e 100, onde 100 é a imagem original e 0 é transparente. Os botões de controle mudam o QLineEdit com incrementos
e decrementos de 1, enquanto é possível editar o campo diretamente para o valor desejado. Foi decidido então armazenar,
para cada imagem, um valor inteiro alpha que representa o brilho atual. Para não precisar sincronizar diversos vetores
e não precisar guardar em memória o tamanho do vetor diversas vezes, o std::vector\<double\> que armazenava o valor do 
fator foi alterado para um std::vector\<DadosImagem\>, atualizado para conter um double fator e um int alpha. Para sanitização
dos valores de entrada, o QLineEdit utilizou de um QIntValidor(0,100).

Para a implementação do brilho em si, foi utilizado um QPainter e um novo QPixmap. Para tanto, o painter tem seu brilho ajustado
de acordo com o valor de alpha da aba atual, e uma chamada para pintar o pixmap atual é realizado, alterando o brilho desta forma.
Como o brilho do pixmap aceita valores apenas entre 0 e 1, o valor armazenado foi escalado em intervalos de 0.01, de forma que 
100 * 0.01 = 1 e 0 * 0.01 = 0. Isso causou um problema, com alterações no brilho afetando futuras chamadas para esse método. 
Por exemplo, caso a imagem fosse alterada para opaca, não seria possível restaurar a imagem original. Para lidar com esse problema,
a struct DadosImagem foi alterada para armazenar o QPixmap original, sendo agora todas as operações de brilho realizadas em cima dele.

Por fim, foram adicionadas operações para cuidar dos controles de brilho, desabilitando os controles pelos botões respectivos caso o brilho 
esteja no máximo ou mínimo. Isso também é realizado na alteração de aba e na remoção de aba.

### Rotação

Por último, para realizar a rotação, foram implementados apenas 2 QPushButtons que realizam a operação no fator de rotação, incrementando
e decrementando em 90 mod 360. A função de aplicar o brilho então foi alterada para aplicar brilho e rotação, e o QPainter foi atualizado 
para aplicar duas translações para manter a rotação centralizada, com uma rotação entre essas operações.

## Possíveis melhorias

Como possíveis melhorias, pode-se mencionar:

- Adicionar funcionalidade de importar imagens pelo clipboard
- Criar classes novas representando a QTabWidget e esses atributos
- Melhorar usabilidade com shortcuts
- Manter imagem no canto superior esquerdo após rotação
- Reestruturação do programa em diretóries include, src e lib
- Adicionar testes unitários com GoogleTest.

Importando imagens pelo clipboard, tem-se uma melhoria na usabilidade do programa. A criação de novas classes pode facilitar na manutenção e
extensibilidade, como também com performance. Outra possível alternativa seria de armazenar um index diretamente na classe, de forma
a diminuir os queries a QTabWidget. por último, shortcuts deixam as operações muito mais rápidas de serem realizadas pelo usuário, economizando
em cliques. 

Também é necessário reestruturar os arquivos, no caso do programa crescer de tamanho. Devido ao tamanho atual pequeno, não foi visto a necessidade
de fazer isso em um primeiro momento.

## Resultado

Acredito que o resultado tenha sido bom, com uma boa oportunidade de aprendizado de manipulação de imagem utilizando Qt, sendo necessário ainda
algumas melhorias.
