# Trabalho da Disciplina Redes de Computadores (SSC0142)
## Módulo 1 - Implementação de Sockets

## Grupo
- Fabiana Dalacqua Mendes NUSP: 9894536
- Gyovana Mayara Moriyama NUSP: 10734387
- Henrique Matarazo Camillo NUSP: 10294943

## Instruções para uso
- "make" para compilar o código.
- "make run_server" para executar o servidor, e "make run_client" para executar o cliente. Obs: o servidor deve ser executado antes do cliente.

## Comandos 
### Cliente:
- /connect conecta o cliente com o servidor
- /nickname NomeUsuario faz com que o usuário seja reconhecido pelo nome especificado
- /ping o servidor responde com pong
- /join NomeCanal entra no canal com o nome especificado
- /help mostra os comandos que o usuário pode utilizar
- /quit desconecta o cliente do servidor
#### Administrador:
- /whois NomeUsuario manda para o administrador o IP do usuário especificado
- /mute NomeUsuario muta o usuário especificado
- /unmute NomeUsuario desmuta o usuário especificado
- /kick NomeUsuario retira o usuário especificado do canal
### Servidor:
- /quit desconecta todos os clientes que ainda estiverem conectados e fecha o servidor
