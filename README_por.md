<p align="center">
  <img src="images/ARMOR_BANNER.svg" alt="ARMOR-NODE-ELECTRICAL banner" width="100%">
</p>

# ⚡ ARMOR-NODE-ELECTRICAL

<p align="center">
  <a href="README.md">🇺🇸 English</a> |
  <a href="README_spa.md">🇪🇸 Español</a> |
  <a href="README_fra.md">🇫🇷 Français</a> |
  <a href="README_ita.md">🇮🇹 Italiano</a> |
  <a href="README_deu.md">🇩🇪 Deutsch</a> |
  <a href="README_zho.md">🇨🇳 简体中文</a> |
  <a href="README_jpn.md">🇯🇵 日本語</a> |
  🇵🇹 <b>Português</b>
</p>

### Nó elétrico: lê as tensões, correntes, potência e energia da rede AC e DC da casa e guarda as regras para a comutar (núcleo testado em anfitrião e um firmware que compila; nunca correu numa placa)

<p align="center">
  <img src="https://img.shields.io/badge/License-GPL%203.0--or--later-blue.svg" alt="GPL 3.0 or later">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599c.svg" alt="Language">
  <img src="https://img.shields.io/badge/Meters-PZEM--004T%20%2F%20017-ffb020.svg" alt="Meters">
  <img src="https://img.shields.io/badge/Checks-376%2C195-2ea44f.svg" alt="Checks">
  <img src="https://img.shields.io/badge/Tests-382264%20checks-2ea44f.svg" alt="Tests">
  <img src="https://img.shields.io/badge/Version-0.5.2-0f6cbd.svg" alt="Version">
  <img src="https://img.shields.io/badge/Maturity-functional-00E5FF.svg" alt="Maturity">
</p>

---

**Verificação de honestidade - o que funciona hoje:** O núcleo (as tramas dos contadores PZEM-004T v3 e PZEM-017, o barramento que os interroga, a mensagem do nó, as entradas analógicas e as regras de comutação: 376 195 verificações em 11 programas de teste, 375 008 delas sobre as regras de comutação) é testado num computador com contadores e contactores de substituição, e as mensagens que produz são aceites pelo ARMOR-COMMON. O firmware do nó (as definições, a linha série, as páginas Contadores e Leituras do seu painel, Bluetooth, atualizações assinadas) compila no contentor ESP-IDF 5.5.5 para as duas placas. **Nunca correu numa placa, nada foi ligado a um contador nem a um contactor, e nada aqui comuta coisa alguma.**

---

## 📋 Num relance

| | |
|---|---|
| Versão | `0.5.2` |
| Maturidade | Funcional |
| Função | Nó de campo elétrico: contadores, entradas e manobra (família: Field Nodes, pai: ARMOR-COMMON) |
| Tecnologia | C++17, ESP-IDF 5.5.5, ESP32-S3 |
| Testes | 382264 verificações em 19 programas de anfitrião e 5 em Python (`ctest`) |
| Equipamentos | Contadores PZEM-004T v3 e PZEM-017 (até 16), entradas analógicas, relés; manobra desligada por omissão |
| Funciona em | Uma placa ESP32-S3 (Ethernet ou Wi-Fi); testes de anfitrião em qualquer computador |
| Em hardware real | Nenhum ainda: nenhuma placa montada nem contador ou contactor ligados; nada comuta coisa alguma |
| Licença | GPL-3.0-or-later |

**Ir para:** [Visão geral](#-visão-geral) · [Arquitetura](#-arquitetura) · [Segurança](#-segurança-e-proteção) · [Interfaces](#-interfaces) · [Configuração](#-configuração) · [Início rápido](#-início-rápido) · [Testes](#-testes) · [Problemas](#-resolução-de-problemas) · [Estrutura](#-estrutura-do-repositório) · [Relacionados](#-projetos-relacionados)

## 🚦 Estado do projeto

Cada linha é uma parte deste repositório e a luz diz até onde chega hoje. É escrita com o que o repositório consegue demonstrar (testes, compilações, execuções), não com o que está previsto.

🟢 Feito e verificado · 🟡 Funciona, com limites conhecidos · 🟠 Feito a meio ou ainda por verificar · 🔴 Por fazer ou a não funcionar

| Área | Estado | O que significa aqui |
|---|---|---|
| Software | 🟢 | O firmware compila no contentor ESP-IDF 5.5.5 para as duas placas (Wi-Fi e Ethernet). |
| Testes | 🟢 | Passam 382.264 verificações em 19 programas de anfitrião (`ctest`), 375.036 delas sobre as regras de manobra; os contadores e contactores são simulações. |
| Protocolos e dados | 🟡 | As tramas e leituras dos PZEM-004T v3 e PZEM-017 são verificadas contra as tramas documentadas e amostras gravadas; nenhum contador respondeu ainda. |
| Interface | 🟡 | As páginas de contadores e leituras do painel web foram exercitadas num navegador contra um nó simulado, não numa placa. |
| Hardware | 🔴 | Nenhuma placa construída nem contador ou contactor ligados: nada funcionou ainda em equipamentos reais. |
| Segurança | 🟡 | O código de configuração, o acesso com limite de palavras-passe erradas, o PIN Bluetooth e as atualizações assinadas estão testados no anfitrião; não houve revisão independente. |
| Atualizações | 🟠 | As verificações de hash e de assinatura Ed25519 e a lógica de retrocesso passam no anfitrião; uma atualização pelo ar nunca foi executada numa placa. |
| Escrita nos equipamentos | 🟠 | As regras para manobrar contactores só foram testadas contra contactores simulados; nada aqui manobrou ainda nada real. |
| Integração | 🟠 | As mensagens do nó são validadas contra os esquemas partilhados e os vetores de conformidade do ARMOR-COMMON. O seu cliente MQTT é exercitado contra um Mosquitto real nos testes; nenhuma placa os enviou ainda a um servidor. |
| Documentação | 🟢 | Oito idiomas, o guia do firmware, as notas de cablagem e o registo de alterações estão atualizados. |

## 🎯 Visão geral

* **Ler os contadores:** as tramas Modbus RTU dos **PZEM-004T v3** (AC) e **PZEM-017** (DC) da Peacefair: o CRC, o pedido que lê os registos e a descodificação da resposta (tensão, corrente, potência, energia, frequência, fator de potência, alarme). Só se constrói o pedido de leitura: os que escrevem num contador (o seu endereço, os seus limiares, a reposição da sua energia) não se constroem em lado nenhum. Uma resposta que não encaixa, ou que traz um valor que nenhum contador real poderia dar, recusa-se.
* **O barramento:** uma linha com vários contadores consulta-se um de cada vez, com um limite de tempo; guarda-se a última boa leitura de cada um e um contador que fica em silêncio deixa de se publicar após dez segundos.
* **A mensagem** `armor/electrical/<node>/state`: uma entrada por canal (um circuito, uma linha, a entrada da rede, um barramento DC) com AC ou DC, tensão, corrente, potência, energia, frequência, fator de potência, o estado de um interruptor como o nó o vê e um alarme; está no contrato partilhado e leva um estado, nunca um comando.
* **As regras de comutação,** à parte de qualquer hardware: um controlador para a transferência entre duas fontes que nunca comanda as duas, precisa de a comutação estar permitida, do nó armado mesmo antes, de ambos os contactores confirmados abertos durante todo o tempo morto, e converte um contactor que não mostra o que lhe foi dito numa avaria que fica até ser reconhecida. **A comutação está desativada por omissão e nada aciona hardware algum.**
* **Os comandos a um interruptor, não ativados:** a mensagem de estado pode levar o estado dos interruptores do nó, e o contrato partilhado tem um comando (`arm`, depois `close_a` ou `close_b` com um token de uso único, `open`, `acknowledge`) e a resposta do nó; o núcleo lê-os e responde-lhes com as regras acima, testado contra os vetores partilhados e com contactores substitutos. Nenhuma imagem de firmware o inclui, por isso nada pode acionar um contactor, e o ARMOR-SERVER recusa enviar um comando a menos que se ligue (`ARMOR_ELECTRICAL_SWITCHING=1`) e a ACL do broker o permita.
* **Onde se mostra:** o *Desenhador elétrico* do ARMOR-STUDIO desenha a rede da casa e mostra em cada elemento o que o seu canal mede; o ARMOR-SERVER guarda as leituras, o seu histórico e as somas.
* **O firmware do nó** (ESP32-S3-WROOM-1 N16R8 em Wi-Fi, ou a Waveshare ESP32-S3-ETH no seu cabo): as definições, uma linha série para até dezasseis contadores e o painel web dos outros nós (configuração, utilizadores, Wi-Fi, broker, atualização pelo ar, HTTPS) com as suas próprias páginas Contadores e Leituras, em oito idiomas. Só lê: as regras de comutação não estão ligadas nele. Veja [o firmware](docs/NODE_FIRMWARE.md).
* **Configuração a partir de um telemóvel por Bluetooth,** o mesmo canal que o do nó de radar: a aplicação ARMOR encontra o nó como `ARMOR-XXXXXX` e define o seu nome, Wi-Fi, endereço, broker e modo Bluetooth com os utilizadores do painel e o código de configuração ([o protocolo](docs/BLE_PROVISIONING.md)). Só escuta enquanto o nó não tem utilizador, a menos que se diga o contrário. O lado de rádio nunca correu numa placa.
* **Atualizações pelo ar assinadas:** o nó só instala do GitHub uma imagem cujo SHA-256 coincide e que leva uma assinatura Ed25519 válida do responsável (a chave pública está no firmware, a privada nunca sai do responsável); um ficheiro carregado à mão por um administrador no painel continua a ser aceite. As páginas e mensagens do painel estão em oito idiomas.
* **Emparelhamento Bluetooth com PIN:** o PIN de seis dígitos deriva do código de configuração, é impresso na consola USB e mostrado a um administrador no painel; o telemóvel tem de o introduzir antes de poder escrever as definições do nó.
* **Ainda não:** o firmware a correr numa placa, o hardware, qualquer medição de uma instalação real e qualquer comutação.
* **Só a casa pode usar o painel:** o painel responde por omissão à rede privada (ou às redes que o dono indicar, como uma VPN), recusa uma lista que deixaria de fora quem a guarda, conta as palavras-passe erradas por pessoa e por endereço e funciona atrás do proxy do servidor para ser alcançado de fora de casa sem publicar o nó.
* **As estimativas dizem-no:** um canal feito a partir de uma entrada de corrente leva `power_estimated`, por isso o Studio e o servidor mostram a sua potência como estimada (potência aparente a partir de uma corrente e uma tensão nominal) e marcam um valor de rede que inclua uma.
* **Ligação cifrada opcional ao broker (TLS):** desativada por predefinição, por isso o nó fala MQTT sem cifra como antes. Um interruptor e uma caixa para o certificado da autoridade própria da instalação (PEM, no máximo 2 KB) na página Broker fazem-no ligar-se com `mqtts://`, confiar só nessa autoridade e verificar que o certificado do broker foi feito para o endereço alcançado; o que não for um único certificado PEM é recusado. Coberto por testes no computador; o handshake ainda não foi executado numa placa.
* **Relatório de saúde:** cerca de uma vez por minuto o nó publica há quanto tempo está ligado e porque reiniciou da última vez, a memória livre, a ligação e o sinal Wi-Fi, quantas vezes se ligou ao broker e o último erro que registou (`armor/health/<nó>`, o contrato `node_health` do ARMOR-COMMON). O código que o constrói é partilhado por todos os nós e testado num computador; nunca correu numa placa.
* **Controlo a partir do servidor:** o Studio pode reiniciar o nó de forma limpa, pô-lo em pausa até uma hora (retoma sozinho), pôr o seu trabalho em espera (sem fim, mantida nos reinícios, o nó continua ligado e a responder), fazê-lo mostrar-se, pedir um instantâneo de diagnóstico e marcá-lo em manutenção (`armor/control/<nó>/...`, o contrato `node_control` do ARMOR-COMMON). Cada ordem tem dois passos com um token de uso único que pertence a essa operação, só o servidor as pode escrever no broker, e não existe reposição de fábrica, apagamento nem sono profundo. Enquanto o nó está em pausa ou em espera, todos os relés ficam abertos e nenhum se move, peça quem pedir (o painel, o broker ou uma central); os contadores continuam a ser lidos. O código é partilhado por todos os nós e testado num computador; ainda não correu numa placa.
* **Leituras que dizem quão frescas são:** cada canal de contador leva a idade da sua leitura e quantos pedidos ao contador falharam, pelo que o servidor distingue um 0 W real de um contador que deixou de responder; e uma tarefa de contadores bloqueada reinicia a placa através do watchdog.
* **Diagnóstico sem cabo:** depois de um arranque que não foi limpo (um bloqueio, um watchdog, uma quebra de alimentação, uma reserva de memória falhada), o relatório de saúde diz o motivo durante dez minutos; uma falha deixa o seu core dump para ver, descarregar ou apagar a partir do painel; as definições guardadas são copiadas antes de cada instalação de firmware e recuperadas se uma atualização as danificar; e um administrador pode pedir ao nó, através do servidor, um pacote de diagnóstico ou o seu registo com um token de utilização única.
* **Intertravamento de relés:** um relé pode indicar outro que nunca deve estar fechado ao mesmo tempo (as duas fontes de uma carga, os dois sentidos de um motor); pedir para fechar um enquanto o outro está fechado é recusado, venha do painel, do broker ou da central, e o nó nunca abre o outro por si. Testado num computador; não executado numa placa.
* **Leitura, apenas de leitura, das definições próprias dos contadores:** após a primeira leitura boa e a cada `bus.check_settings_s` (600 s, 0 = nunca) o nó lê os registos holding do PZEM (004T v3: limiar de potência e endereço; 017: limiares de tensão, endereço e shunt) no mesmo turno das medidas, e avisa se o endereço guardado não é o que usa; não se escreve nada no contador.
* **Selagem opcional dos segredos guardados:** desativada por predefinição (`security.seal_secrets`, ou `nodes.nvs_encryption` do servidor). Ativada, as palavras-passe, os tokens, as chaves, os PIN e os hashes de palavras-passe que o nó guarda ficam selados com ChaCha20-Poly1305 e tudo o resto continua legível; ativá-la ou desativá-la reescreve-os sem perder nada, mesmo que o nó pare a meio. Não se queima nenhum eFuse nem se cifra a flash: a chave está numa partição do mesmo chip, pelo que **não** protege contra quem tenha a placa, e um nó instalado antes dessa partição mantém os segredos como estavam até ser gravado com a nova tabela de partições.

## 🧩 Papel no A.R.M.O.R.

O ARMOR-NODE-ELECTRICAL mede a rede CA e CC da casa com os seus contadores e guarda as regras para comutar entre fontes. Publica as leituras para o [ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER), que guarda o histórico e a energia de cada dia e gera os alarmes; o menu *Elétrico* e o *Desenhador elétrico* do [ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO) e o ecrã elétrico do [ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL) mostram-no. A comutação está desenhada (armar, um token de uso único, o intertravamento, a confirmação dos dois contactores) mas não ativada.

| Nome | Como | Para quê |
|---|---|---|
| ARMOR-SERVER | MQTT `armor/electrical/<id>/{state,command,result}` | Recebe os canais e ordena uma transferência (só se a manobra estiver ativa em três sítios) |
| ARMOR-STUDIO | menu Elétrico, Electrical Designer | Mostra os canais e os alarmes e abre o painel do nó |
| ARMOR-ANDROID-CONTROL | Bluetooth | Configura o nó no terreno e mostra os canais |
| ARMOR-COMMON | contrato elétrico | As mensagens são verificadas com os seus esquemas; o firmware comum é copiado dele |

## 🏛️ Arquitetura

<p align="center">
  <img src="images/architecture.svg" alt="Arquitetura de ARMOR-NODE-ELECTRICAL: quem o chama, o que contém e de que depende" width="100%">
</p>

## 🖼️ Capturas de ecrã

Capturadas a 1920×1080 numa bancada de testes, com dados de exemplo e sem qualquer instalação real por trás.

<table>
<tr><td width="50%" valign="top"><img src="images/screenshots/overview.png" alt="Resumo: o estado do nó num relance - rede, broker, firmware e o que reporta." width="100%"><br><sub>Resumo: o estado do nó num relance - rede, broker, firmware e o que reporta.</sub></td><td width="50%" valign="top"><img src="images/screenshots/meters.png" alt="Contadores: os contadores de energia e canais analógicos que o nó lê, cada um com o seu nome e escala." width="100%"><br><sub>Contadores: os contadores de energia e canais analógicos que o nó lê, cada um com o seu nome e escala.</sub></td></tr>
<tr><td width="50%" valign="top"><img src="images/screenshots/readings.png" alt="Leituras: o que o nó está a ler neste momento, com a idade de cada valor." width="100%"><br><sub>Leituras: o que o nó está a ler neste momento, com a idade de cada valor.</sub></td><td width="50%" valign="top"><img src="images/screenshots/zigbee.png" alt="Rádio Zigbee: a ponte série para o módulo coordenador, que cliente a pode usar e o estado da ponte." width="100%"><br><sub>Rádio Zigbee: a ponte série para o módulo coordenador, que cliente a pode usar e o estado da ponte.</sub></td></tr>
</table>

## 🔒 Segurança e proteção

| Ameaça | Defesa |
|---|---|
| Palavras-passe em claro ou compiladas | PBKDF2-HMAC-SHA256 com 10 000 rondas; nenhuma palavra-passe no firmware (um nó sem utilizador entra em configuração); no máximo 4 utilizadores (admin ou visualizador) e de 8 a 64 caracteres; no máximo 6 sessões, 30 minutos de inatividade |
| Tomar um nó acabado de gravar | A configuração exige um código de arranque: o da compilação (`CONFIG_ARMOR_SETUP_CODE`) ou um aleatório guardado na flash e mostrado na consola USB; com segredo de frota é `HMAC-SHA256(segredo, MAC)`; é também a chave do Wi-Fi de configuração (WPA2, nunca em ponte) |
| O painel alcançado a partir da internet | Responde por omissão às redes privadas de uma casa (10/8, 172.16/12, 192.168/16, 169.254/16, 127/8 e 100.64/10 onde vivem algumas VPN); a definição `web.admin_sources` lista outros endereços e redes IPv4 (8 entradas, 120 caracteres); tudo o que está fora é recusado exceto a identidade pública `GET /api/v1/session`; uma lista que deixaria de fora quem a guarda é rejeitada |
| O painel em claro por HTTP | HTTPS no 443 com um certificado que o nó cria sozinho (autoassinado, na flash, impressão digital SHA-256 visível para comparar); o painel pode ser http, https ou ambos; o cookie de sessão é sempre HttpOnly e SameSite=Strict (e Secure por TLS) |
| Um firmware falso do GitHub | Uma imagem só se instala com um SHA-256 coincidente e uma assinatura Ed25519 válida feita com a chave do mantenedor (`no_checksum`, `no_signature`, `bad_signature`, `checksum_mismatch`, `wrong_firmware`, `not_newer`, `version_mismatch` recusam-na); um envio manual por um administrador não é verificado: um limite explícito |
| Sequestro da configuração Bluetooth | Um PIN de 6 dígitos derivado do código de configuração, visível na consola USB e ao administrador; o telemóvel tem de o introduzir antes de escrever definições; a janela dura `CONFIG_ARMOR_BLE_SETUP_MINUTES` (10 por omissão, de 1 a 120) |
| Uma ordem remota que faz mal | Dois passos com um token de uso único ligado a essa operação; só o servidor pode escrever esses tópicos no broker; uma pausa dura uma hora no máximo; não há reposição de fábrica, apagamento nem sono profundo por ordem remota |
| Segredos guardados lidos por quem tem a placa | Opcional (desligado por omissão, interruptor `security.seal_secrets` ou `nodes.nvs_encryption` do servidor): ChaCha20-Poly1305 para palavras-passe, tokens, chaves, PIN e hashes. Não cifra a flash nem queima um eFuse: a chave está numa partição do mesmo chip, por isso não protege contra quem tem a placa |
| A ligação ao broker em claro | TLS opcional (`mqtts://`) com a autoridade própria do dono (PEM até 2 KB) e verificação do nome; desligado por omissão. O handshake só correu em testes de anfitrião, não numa placa |
| Rede ou contínua perigosa na placa | O nó só lê os módulos PZEM (módulos fechados, linha série isolada) e nunca leva rede na sua placa; o que comuta é um andar separado testado em bancada com uma lâmpada e um transformador de isolamento |
| Duas fontes fechadas ao mesmo tempo | O controlador de transferência nunca ordena as duas: exige manobra permitida, nó armado mesmo antes e ambos os contactores confirmados abertos durante todo o tempo morto (`dead_time_ms` 2000 por omissão); um contactor que não mostra o que foi ordenado é uma falha que fica até ser reconhecida (`kDidNotOpen`, `kDidNotClose`); continua a ser obrigatório um bloqueio mecânico entre os contactores |
| Uma ordem repetida, ou uma manobra que ninguém pediu | `arm` dá um token de uso único de 64 bits que `close_a` ou `close_b` têm de levar, dentro de `arm_window_ms`; as ordens nunca são retidas; no máximo 4 interruptores; a manobra está DESLIGADA a não ser que o firmware a permita (`switching_enabled`), o servidor corra com `ARMOR_ELECTRICAL_SWITCHING=1` e a lista de acesso do broker esteja aberta para esse nó |
| Uma falha, um retorno perdido ou a manobra desligada | Cada controlador responde com TODAS as bobines abertas (falha => aberto; se aberto é seguro para a carga decide a instalação); o estado na mensagem é o que o contacto auxiliar mostra, não o que foi ordenado; os pares de relés estão bloqueados e o nó nunca abre o par por si mesmo |
| Uma leitura velha mostrada como real | Um contador mudo deixa de ser publicado após 10 s; cada canal leva a idade da sua leitura e o número de pedidos falhados; uma tarefa de contadores bloqueada reinicia a placa por watchdog; a cada `bus.check_settings_s` (600 s, 0 = nunca) as definições dos contadores são lidas (só leitura) e um endereço alterado é comunicado |

A rede (230 V CA) e a corrente contínua de um banco de baterias podem matar e provocar incêndios. [docs/SAFETY.md](docs/SAFETY.md) diz o que o desenho pressupõe; não é o desenho de uma instalação e não substitui a regulamentação do país nem um instalador qualificado. Hoje nada disto comuta nada.

## 🔌 Interfaces

| Parte | O que define |
|---|---|
| MQTT | `armor/electrical/<id>/state` (um canal por entrada: CA ou CC, tensão, corrente, potência, energia, frequência, fator de potência, estado do interruptor, alarme; contrato do ARMOR-COMMON), `armor/electrical/<id>/command` e `/result`, `armor/device/<id>/<relé>/{state,set}`, `armor/health/<id>`, `armor/control/<id>/...` |
| Ordens | `arm`, `close_a`, `close_b` (com o token), `open` e `acknowledge` (limpa uma falha) |
| Contadores | Peacefair PZEM-004T v3 (CA) e PZEM-017 (CC) por Modbus RTU, até 16 numa linha série; só o pedido de leitura é construído (o que escreve endereço ou limiares não existe); entradas analógicas, contactos e relés na placa base; uma ponte Zigbee ([docs/ZIGBEE.md](docs/ZIGBEE.md)) |

## ⚙️ Configuração

| Variável | Predefinição | O que define |
|---|---|---|
| `bus.check_settings_s` | 600 | Segundos entre as verificações só de leitura das definições dos contadores (0 = nunca) |
| `dead_time_ms`, `confirm_timeout_ms` | 2000 | O tempo morto de uma transferência e quanto tempo tem um contactor para mostrar o que foi ordenado |
| `switching_enabled` | off | A manobra está DESLIGADA a não ser que esta definição, o servidor (`ARMOR_ELECTRICAL_SWITCHING=1`) e a lista de acesso do broker a permitam |
| `CONFIG_ARMOR_NODE_ID`, `_SETUP_CODE`, `_FLEET_SECRET`, `_MQTT_*`, `_HEARTBEAT_S`, `_BLE_SETUP_MINUTES`, `_BOOT_FACTORY_RESET` | - | As definições que todos os nós partilham: o nome (vazio dá `armor-` e os últimos bytes do MAC), o código de configuração (vazio: aleatório), o segredo de frota (16+ caracteres, nunca no repositório), as primeiras definições do broker, o batimento (de 2 a 300 s), os minutos de configuração Bluetooth (de 1 a 120) e a reposição com BOOT (8 s nos primeiros 30 s) |
| `system.login_limit_per_minute` | 30 (0-120) | Palavras-passe erradas por minuto antes de o painel bloquear o início de sessão a todos (0 = sem limite global); lê-se e altera-se no Studio (um botão em cada nó) ou com `POST /api/v1/admin/nodes/settings`, onde o acesso ao painel é usado uma vez e nunca guardado |

## 🚀 Início rápido

**Requisitos:** um compilador C++17 para os testes de anfitrião; o contentor ESP-IDF 5.5.5 para compilar a imagem; uma placa ESP32-S3 (Ethernet ou Wi-Fi).

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host   # host tests, -Werror
node tools/panel_mock.mjs --user admin:adminpass123   # the panel without a board
tools/build_node.sh generic s3-wifi                  # the image in the ESP-IDF container (or s3-eth)
```

Veja o [guia do firmware](docs/NODE_FIRMWARE.md) e o [canal Bluetooth](docs/BLE_PROVISIONING.md).

Veja o [desenho](docs/DESIGN.md), as [notas de segurança](docs/SAFETY.md), as [regras de comutação](docs/SWITCHING.md), os [protocolos](docs/PROTOCOLS.md) e as [mensagens](docs/ELECTRICAL_MESSAGES.md).

## 🧪 Testes

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host
python -I tools/armor_ci_validate.py
```

19 programas de teste de anfitrião e 5 verificações Python correm com `ctest`; 382264 verificações contadas pelos programas de anfitrião, compiladas com `-Werror`.

Verificações Python: os textos do painel em oito línguas, as tarefas, os caminhos de gravação, a sessão Bluetooth sob o seu bloqueio e as rotas que o painel chama (e, em alguns nós, os códigos HTTP ou os glifos dos tipos de letra)

Os testes de anfitrião precisam de qualquer compilador C++17 (Linux, WSL, MSYS2); o painel web também é exercitado num navegador real contra um nó de substituição (`tools/panel_mock.mjs`, `tools/panel_browser_test.mjs`).

A maior parte das verificações são varrimentos exaustivos das regras de manobra (`test_switch_set` e `test_interlock`: 375 036); os contadores e contactores são substitutos. O `test_clock` só imprime `ok`.

## 🏗️ Compilação e publicação

`tools/build_node.sh` compila a imagem no contentor ESP-IDF 5.5.5 e a versão leva a imagem, o seu SHA-256 e a sua assinatura Ed25519, uma por placa; a imagem grava-se por USB-C da primeira vez e depois atualiza-se a partir do painel.

## 🩺 Resolução de problemas

| Vê | Porque | Faça |
|---|---|---|
| Perdeu a palavra-passe ou o nó já não se liga à rede | Um nó em configuração e sem endereço após 90 s abre o seu próprio Wi-Fi `ARMOR-SETUP-xxxxxx` (abra `http://192.168.4.1/`) | Para recomeçar mantenha BOOT 8 segundos nos primeiros 30 segundos após ligar (apaga definições e utilizadores), ou use *Reposição de fábrica* no painel |
| Uma atualização do GitHub é recusada | A versão não traz soma de verificação nem assinatura, não coincidem (`e_no_checksum`, `e_no_signature`, `e_checksum_mismatch`, `e_bad_signature`), não traz imagem para esta placa (`e_no_asset_for_board`) ou é de outro projeto (`e_wrong_firmware`) | O nó não mudou; tente de novo ou deixe um administrador enviar o ficheiro à mão |
| Um interruptor mostra uma falha | O contactor não mostrou aberto ou fechado como ordenado (`kDidNotOpen`, `kDidNotClose`); fica até ser reconhecido | Verifique a cablagem do contacto auxiliar e envie `acknowledge` |
| Uma ordem é recusada | A manobra está desligada (`switching_enabled` falso), o token é inválido ou expirou, ou o nó não estava armado mesmo antes | Arme de novo e envie a ordem com o novo token |

## 📂 Estrutura do repositório

<p align="center">
  <img src="images/repository-structure.svg" alt="Estrutura do repositório de ARMOR-NODE-ELECTRICAL" width="100%">
</p>

<details>
<summary>Versão em texto da árvore</summary>

```text
ARMOR-NODE-ELECTRICAL/
├── main/                                o firmware: um componente ESP-IDF
├── core/                                a lógica sem ESP-IDF, testada num computador
├── panel/                               o painel web embebido no firmware
├── tests/                               testes de anfitrião e verificações Python
├── docs/                                12 guias: segurança, manobra, mensagens, hardware
├── tools/                               ferramentas de compilação, empacotamento e verificação
├── .github/                             fluxo de CI e modelos de issues e pull requests
├── images/                              faixa, ícone e as duas imagens deste README
├── build.sh build.bat                   compilar pelo fluxo de projeto partilhado
├── test.sh test.bat                     executar os testes pelo fluxo partilhado
├── run.sh run.bat                       arrancar pelo fluxo de projeto partilhado
├── armor.project.json                   manifesto: versão, função, maturidade, comando de build
├── CHANGELOG.md                         o que mudou em cada versão
├── SECURITY.md SUPPORT.md LICENSE       política de segurança, apoio e licença
├── CONTRIBUTING.md CODE_OF_CONDUCT.md   como contribuir e o código de conduta
├── CMakeLists.txt                       o projeto ESP-IDF
├── partitions.csv                       a tabela da flash
├── sdkconfig.defaults                   as definições do firmware
├── sdkconfig.board.s3-eth               as definições de uma placa
├── sdkconfig.board.s3-wifi              as definições de uma placa
├── secrets/                             exemplo do ficheiro do nó (os verdadeiros não são versionados)
├── build-test.sh build-test.bat         executar os testes pelo fluxo partilhado
└── .gitattributes .gitignore            definições do Git
```

</details>

## 🗺️ Roteiro e limites conhecidos

**Limites conhecidos hoje**

* As tramas e leituras dos PZEM-004T v3 e PZEM-017 são verificadas contra as tramas documentadas e amostras gravadas; nenhum contador respondeu ainda.
* As páginas de contadores e leituras do painel web foram exercitadas num navegador contra um nó simulado, não numa placa.
* Nenhuma placa construída nem contador ou contactor ligados: nada funcionou ainda em equipamentos reais.
* O código de configuração, o acesso com limite de palavras-passe erradas, o PIN Bluetooth e as atualizações assinadas estão testados no anfitrião; não houve revisão independente.
* As verificações de hash e de assinatura Ed25519 e a lógica de retrocesso passam no anfitrião; uma atualização pelo ar nunca foi executada numa placa.
* As regras para manobrar contactores só foram testadas contra contactores simulados; nada aqui manobrou ainda nada real.

**A seguir**

* Experimentar os contadores, os contactores e as regras de manobra numa bancada com lâmpada e transformador de isolamento, e depois uma placa real com a atualização pelo ar.

## 🌍 Idiomas

O README e os textos existem em inglês, espanhol, francês, italiano, alemão, chinês, japonês e português europeu.

## 🔗 Projetos relacionados

O **A.R.M.O.R.** (Autonomous Radar & Multimodal Observation Range) é um sistema de segurança perimetral feito de repositórios independentes. Cada um tem a sua própria versão, os seus próprios testes e o seu próprio README; esta é a família:

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - Contratos de mensagens, validadores, vetores de conformidade e tipos gerados
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - Firmware de nó de campo para ESP32-S3 com três radares e o seu próprio painel web
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - Protocolos de inversores e baterias solares e as mensagens de um nó porta de ligação
* **ARMOR-NODE-ELECTRICAL** (este repositório) - Nó elétrico: contadores, a mensagem das leituras da rede e as regras de comutação
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - Nó e painel de alarme: zonas, armar, atrasos, sirene e PIN, com o servidor ou sem ele
* **[ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI)** - Painel tátil: o estado do sistema num ecrã de parede, armar e reconhecer, e a casa do assistente de voz
* **[ARMOR-NETWORK](https://github.com/JuanenRac/ARMOR-NETWORK)** - A rede local: os seus dispositivos, a internet e o que muda
* **[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER)** - Coordenador central: telemetria, alarmes, dispositivos, leituras solares e câmaras
* **[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO)** - Consola web: câmaras, radar, alarmes, energia solar e o desenhador 2D/3D do local
* **[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL)** - Cliente de operador Android com radar 2D/3D em direto
* **[ARMOR-IOS-CONTROL](https://github.com/JuanenRac/ARMOR-IOS-CONTROL)** - Cliente de operador para iPhone
* **[ARMOR-SERVER-AI](https://github.com/JuanenRac/ARMOR-SERVER-AI)** - Política de inferência visual que explica as suas decisões e nunca atua
* **[ARMOR-VISION](https://github.com/JuanenRac/ARMOR-VISION)** - Zonas de câmara, máscaras e regras de deteção para o Jetson Orin NX
* **[ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)** - Intenções de voz offline com uma confirmação que não pode ser falsificada
* **[ARMOR-HARDWARE](https://github.com/JuanenRac/ARMOR-HARDWARE)** - Caixas, eletrónica e a matriz de aceitação em bancada
* **[ARMOR-DEVOPS](https://github.com/JuanenRac/ARMOR-DEVOPS)** - Implantação, a bancada de testes do CM5, cópias de segurança e TLS
* **[ARMOR-SIMULATOR](https://github.com/JuanenRac/ARMOR-SIMULATOR)** - Simulador de telemetria offline com falhas repetíveis
* **[ARMOR-UPDATER](https://github.com/JuanenRac/ARMOR-UPDATER)** - Deteta, instala e atualiza os repositórios do próprio ecossistema
* **[ARMOR-DOCS](https://github.com/JuanenRac/ARMOR-DOCS)** - Arquitetura, base de segurança e a matriz de capacidades

## 📚 Documentação

* [Segurança](docs/SAFETY.md) e [manobra](docs/SWITCHING.md): o que o desenho pressupõe e como decorre uma transferência
* [Mensagens](docs/ELECTRICAL_MESSAGES.md), [protocolos](docs/PROTOCOLS.md) e [Zigbee](docs/ZIGBEE.md)
* [Desenho](docs/DESIGN.md), [hardware](docs/HARDWARE.md), [placa base](docs/BASE_BOARD.md) e [firmware do nó](docs/NODE_FIRMWARE.md)
* [Testes de bancada](docs/BENCH_TESTS.md), [código de configuração](docs/SETUP_CODE.md) e [provisionamento Bluetooth](docs/BLE_PROVISIONING.md)
* [Matriz de capacidades: o que está demonstrado e o que não está](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/CAPABILITY_MATRIX.md)
* [Catálogo de projetos: versões e como os repositórios dependem uns dos outros](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/PROJECT_CATALOG.md)
* [Registo de alterações deste repositório](CHANGELOG.md)

## 🤝 Contribuir e comunidade

* Antes de um pull request, execute `ctest --test-dir build/host` and `python -I tools/armor_ci_validate.py`: tem de passar. Consulte [CONTRIBUTING.md](CONTRIBUTING.md) e o [código de conduta](CODE_OF_CONDUCT.md).
* O README existe em oito idiomas e é gerado a partir das fontes do ARMOR-DOCS: uma alteração ao texto faz-se lá.
* Uma alteração a uma mensagem entre componentes começa no ARMOR-COMMON.
* Não descreva como a funcionar numa placa aquilo que só foi compilado.
* Comunique uma vulnerabilidade em privado: [SECURITY.md](SECURITY.md). Perguntas, ideias e relatórios: electrohobby3d@gmail.com ([SUPPORT.md](SUPPORT.md)).

## 👤 AUTOR

**JuanenRac (Electro Hobby 3D)** · electrohobby3d@gmail.com

## 📜 LICENÇA

GPL-3.0-or-later - consulte [LICENSE](LICENSE).
