# Data de Entrega: 26/07 (segunda-feira)
# TODO's:
* (OK) Implementar tutorial 1
* (OK) Implementar tutorial 2
* Implementar estruturas para que o sistema de arquivos lide com arquivos e 
pastas numa árvore e não apenas na pasta raiz
* Implementar uma forma de persistência das informações do sistema de arquivos
* Implementar uma forma do FUSE enviar pacotes via rede
* (OK) Estabelecer uma conexão entre duas máquinas virtuais (postar no readme os passos)
* (WIP) Criar um protocolo de rede para enviar as informações do sistema de arquivos
* Integrar o sistema de arquivos criado com o protocolo de rede
* Estabeler diretrizes para transformar o sistema de arquivos criado num módulo do kernel
* (OK) Montar o relatório 2 com o progresso
* Escrever o relatório 2 com o progresso

# Link Relatório 2
[Link Google Docs](https://docs.google.com/document/d/1mNbMBEn-A_RpxZOaDYWwu5ltN8Jpp0l3cipzWPbyqL8/edit?usp=sharing)

# Índice
1. [TODO](#todos)
2. [Requisitos](#requisitos)
3. [Instruções](#instruções)
5. [Conexão VMs](#conexão-vms)
6. [Referências](#referências)

# Requisitos
Para compilar e executar, precisamos de duas bibliotecas do FUSE:
- fuse  
- libfuse-dev  

Elas podem ser instaladas no Ubuntu via:

    sudo apt-get install [lib]
    
# Instruções
## Compilação
Compilar via make:  

    make  
## Testar
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

### Observação
Agora, para conseguir montar o sistema de arquivos, é necessário ter um outro
terminal rodando o *server-mt* escutando na porta 51511. Para executar isso:

    ./server-mt 51511

Caso isso não seja feito, o *ssfs* irá acusar *connection refused*.

# Conexão VMs
Para conectar duas (ou mais) máquinas virtuais entre si (permitindo troca de pacotes) e ainda mantendo a conexão com a Internet, basta seguir os passos desse vídeo de 5 minutos. Não é difícil, apenas criamos uma _NAT Network_ no VirtualBox (está explicado no vídeo). Para isso criamos uma regra de IP's para as máquinas (também explicado no vídeo). Podemos deixar esse campo em "**10.10.10.0/24**" (sem aspas) para padronizar.

[How to Make Virtual Machines Talk to Each Other in VirtualBox](https://www.youtube.com/watch?v=vReAkOq-59I)

Um teste simples para conferir isso é rodar o 

    ifconfig

na máquina e conferir o IP com o formato 10.10.10.x que lhe foi atribuído. Verificado que ela recebeu corretamente um IP desse, podemos mandar um

    ping 10.10.10.5         // IP da outra máquina, para testar a conexão interna
    ping www.google.com     // Testar conexão com a Internet

em uma máquina e na outra (obviamente trocando os IP's correspondentes).

# Referências
[Writing a Simple Filesystem Using FUSE in C (Tutorial pt.1)](https://www.maastaar.net/fuse/linux/filesystem/c/2016/05/21/writing-a-simple-filesystem-using-fuse/)  
[Writing Less Simple, Yet Stupid Filesystem Using FUSE in C (Tutorial pt.2)](https://www.maastaar.net/fuse/linux/filesystem/c/2019/09/28/writing-less-simple-yet-stupid-filesystem-using-FUSE-in-C/)  
[Materiais e Links Úteis](https://docs.google.com/document/d/18UsBey2D0xK0Cs2s6lnUbjN2LXHZ2yL9lT4s0OJkb68/edit)



