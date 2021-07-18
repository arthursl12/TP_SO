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
    cat file349

Dar um CTRL+C no terminal que montou o sistema de arquivos o desmonta (forçosamente).

### Referências
[Writing a Simple Filesystem Using FUSE in C (Tutorial pt.1)](https://www.maastaar.net/fuse/linux/filesystem/c/2016/05/21/writing-a-simple-filesystem-using-fuse/)  
[Writing Less Simple, Yet Stupid Filesystem Using FUSE in C (Tutorial pt.2)](https://www.maastaar.net/fuse/linux/filesystem/c/2019/09/28/writing-less-simple-yet-stupid-filesystem-using-FUSE-in-C/)

Obs.: implementado até o momento apenas o tutorial 1



