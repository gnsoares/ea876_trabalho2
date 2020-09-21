## EA876 Trabalho 2 - Gustavo Nascimento Soares 217530
### Instruções de uso
----

### Compilação

Use: `$ make`

### Execução

O comando de execução precisa de 3 argumentos:

`$ ./main <path> <N> <method>`

em que:

* path: endereço da imagem;<br>
* N: intensidade do efeito de blur;<br>
* method:<br>
    * sync: código síncrono;<br>
    * proc: multiprocesso;<br>
    * thrd: multithread.<br>

### Saída

O arquivo final será salvo no mesmo diretório da entrada com '-out' no final do nome do arquivo.

Exemplo:

| Entrada | Saída |
| ------- | ----- |
| `path/image.jpg` | `path/image-out.jpg` |

### Geração da figura de análise de tempo

Usando `$ make test <path>`, será rodade um script em python que gera uma imagem em pdf com a análise estatística do tempo levado em cada execução para a imagem localizada em `path`. O script roda o programa 100 vezes para cada método de execucão e para 3 intensidades diferentes: N = {2, 3, 5}.

A figura gerada é salva no diretório em que o script é executado como `figure.pdf`.
