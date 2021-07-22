## Data de Entrega: 26/07 (segunda-feira)
## TODO's:
* (OK) Implementar tutorial 1
* (OK) Implementar tutorial 2
* Implementar estruturas para que o sistema de arquivos lide com arquivos e 
pastas numa árvore e não apenas na pasta raiz
* Implementar uma forma de persistência das informações do sistema de arquivos
* Implementar uma forma do FUSE enviar pacotes via rede
* Estabelecer uma conexão entre duas máquinas virtuais (postar no readme os passos)
* Criar um protocolo de rede para enviar as informações do sistema de arquivos
* Integrar o sistema de arquivos criado com o protocolo de rede
* Estabeler diretrizes para transformar o sistema de arquivos criado num módulo do kernel
* Montar o relatório 2 com o progresso

## Requisitos
Para compilar e executar, precisamos de duas bibliotecas do FUSE:
- fuse  
- libfuse-dev  

Elas podem ser instaladas no Ubuntu via:

    sudo apt-get install [lib]
    
## Instruções
### Compilação
Compilar via make:  

    make  
### Testar
Para testar o sistema de arquivos, primeiro temos que montá-lo em algum lugar:

    ./ssfs -f [mount point]

O -f é para termos prints de debug  
Agora, basta ir, num outro terminal, para onde foi montado e testar os (poucos) comandos já implementados

    cd [mount point]  
    ls
    mkdir foo1
    echo "blablabla" > foo2.txt
    cat foo2.txt
    cd foo1
    echo "blabla2" > foo3.txt

Dar um CTRL+C no terminal que montou o sistema de arquivos o desmonta (forçosamente).

## Referências
[Writing a Simple Filesystem Using FUSE in C (Tutorial pt.1)](https://www.maastaar.net/fuse/linux/filesystem/c/2016/05/21/writing-a-simple-filesystem-using-fuse/)  
[Writing Less Simple, Yet Stupid Filesystem Using FUSE in C (Tutorial pt.2)](https://www.maastaar.net/fuse/linux/filesystem/c/2019/09/28/writing-less-simple-yet-stupid-filesystem-using-FUSE-in-C/)  
[Materiais e Links Úteis](https://docs.google.com/document/d/18UsBey2D0xK0Cs2s6lnUbjN2LXHZ2yL9lT4s0OJkb68/edit)



