# Instalação do Proteus para simular o projeto

- Link para download do proteus: https://drive.google.com/file/d/16nJvHS9iWEzDs2bpwx0Q9WYgh32jVYfc/view
- Instalar normalmente
- Depois de instalar, ir na pasta "C:\Program Files (x86)\Labcenter Electronics\Proteus 8 Professional\BIN" e procurar o arquivo "PDS.exe"
- Clicar com o botão direito e ir em propriedades
- Em compatibilidade, ativar compatibilidade com Windows 7 e clicar em abrir como administrador

# Configurar o arduino para o proteus
- Abrir a pasta "ARD_VSM" baixada junto com o proteus
- Ir em "VSM_3D"
- Copiar todos os arquivos e colar em "C:\Program Files (x86)\Labcenter Electronics\Proteus 8 Professional\LIBRARY"

# Fazer o tunelamento de serial entre 2 softwares no PC (direciona uma porta serial pra outra). Com isso é possivel rodar o servidor numa porta serial que vai estar ligada a porta serial do proteus.
- baixar https://eterlogic.com/Downloads.html 64 bits
- instalar normalmente
- é pago, mas da pra usar a versão free limitada
- ir em Create new Device
- Device Type: Virtual Pair
- Selecionar 2 portas COM. uma delas tem que ser a COM1 pq é usada no Proteus. A outra pode ser a COM2 por ex (pro sercidor). Se nao der a COM1, da pra mudar no COMPIM do Proteus.
- Start. Nesse momento o tunelamento da COM1 pra COM2 ja ta feito.
- Abrir o Banrisufrgs e colocar a COM2.

# Carregar o codigo no projeto
- Abrir o projeto "simulation.pdsprj"
- Clicar duas vezes no arduino e em program file procurar pelo arquivo "Projeto1.hex" na pasta e debug do projeto.
- Iniciar simulação clicando no play no canto inferior esquerdo.
- Cada vez que buildar o codigo, parar e iniciar a simulação.