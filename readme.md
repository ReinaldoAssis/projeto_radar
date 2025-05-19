# Especificação do projeto

Proposta de Projeto: Radar Eletrônico Simplificado
Disciplina: Sistemas Embarcados
Plataforma: qemu_cortex_m3 (Zephyr RTOS)
Equipe: em dupla
Data: 15 de maio de 20251. 

Introdução e Objetivos
Este projeto visa a implementação de um sistema de radar eletrônico simplificado utilizando o Zephyr RTOS na plataforma emulada qemu_cortex_m3. O objetivo principal é consolidar conhecimentos em sistemas embarcados, incluindo o uso de RTOS (multithreading, sincronização, comunicação inter-tarefas), drivers de dispositivos (GPIO, Display, Ethernet), configuração via Kconfig e integração com serviços de rede (NTP, envio de dados para a nuvem).

O sistema simulará a detecção de velocidade de veículos através de sensores no solo, exibirá a velocidade e status de infração em um display virtual, e reportará infrações com dados relevantes (incluindo placa veicular simulada) para um serviço web na nuvem com data  data e hora obtidas via NTP.

2. Descrição Funcional
O radar eletrônico proposto terá as seguintes funcionalidades:
Detecção de Passagem e Velocidade: Utilizará dois sensores magnéticos simulados (GPIOs 5 e 6 no QEMU) com um afastamento configurável (Kconfig) para detectar a passagem de um veículo. A velocidade será calculada com base na distância entre os sensores e o tempo decorrido entre as ativações do primeiro e do segundo sensor.
Detecção de Infração: Comparará a velocidade calculada com um limite de velocidade configurável (Kconfig).
Exibição no Visor: Utilizará o driver "Display Dummy" do Zephyr para exibir a velocidade atual do veículo detectado e indicar claramente se ocorreu uma infração por excesso de velocidade.
Captura de Dados do Veículo (Simulada): Em caso de infração, um sistema de câmera simulado será acionado. Este sistema retornará (via ZBUS) uma placa veicular (formato Mercosul) e um hash da foto.
Validação de Placa: A placa retornada pela câmera simulada deve ser validada para garantir que segue o formato Mercosul (Brasil, Argentina, Paraguai). O sistema de câmera simulará uma falha em 10% dos casos, retornando uma placa inválida. Apenas placas válidas serão consideradas para reportar a infração.
Obtenção de Data e Hora (NTP): O sistema deve se conectar a um servidor NTP para obter a data e hora precisas para registrar a infração.
Envio de Dados para a Nuvem: Em caso de infração com placa válida, os dados da infração (data, hora, placa, tipo de veículo, hash da foto) serão enviados para um serviço na nuvem através da interface Ethernet.
3. Design Técnico
Plataforma: qemu_cortex_m3.
Sistema Operacional: Zephyr RTOS.
Drivers/APIs do Zephyr: GPIO, Display, Ethernet, Rede (Sockets, NTP), ZBUS, Kconfig.
Arquitetura de Software: O sistema será estruturado em múltiplos threads concorrentes para gerenciar as diferentes funcionalidades de forma assíncrona:
Thread Principal / Controle: Orquestração geral, recebimento de dados de velocidade, detecção de infração, acionamento da câmera, validação da placa e envio de dados para a thread de rede.
Thread de Sensores: Monitoramento dos GPIOs dos sensores. Utilizará interrupts para capturar timestamps precisos na detecção de bordas (indicação de passagem da roda). Calculará o tempo entre as ativações dos sensores e enviará este dado para a Thread Principal.
Thread de Display: Responsável por atualizar periodicamente o conteúdo exibido no visor virtual com a velocidade atual, limite e status de infração, utilizando a API do Display driver.
Thread de Rede: Gerenciamento da conexão de rede, execução do cliente NTP para sincronização de hora/data e envio dos dados formatados da infração para o serviço na nuvem (e.g., via HTTP POST).
Thread de Simulação da Câmera/LPR: Aguarda por um trigger via ZBUS. Ao receber o trigger, simula um tempo de processamento e gera uma placa veicular (Mercosul válida ou inválida, com probabilidade configurável) e um hash de foto. Publica os resultados em um canal ZBUS de conclusão.

Comunicação Inter-Threads:
Utilização de ZBUS para comunicação entre a Thread Principal e a Thread de Simulação da Câmera (trigger e retorno de resultados).
Outros mecanismos do Zephyr como filas de mensagens ou semáforos podem ser utilizados para comunicação de dados de velocidade e infração entre as threads, conforme necessário.

Simulação de Hardware: Os sensores serão simulados por eventos de mudança de estado em GPIOs específicos no QEMU. A câmera será totalmente simulada em software, utilizando ZBUS para sua interface. O display será o "Display Dummy" do Zephyr.
Networking: Será configurado o stack TCP/IP do Zephyr. Um cliente NTP será utilizad através de biblioteca já disponível. Para o envio para a nuvem, será escolhido um protocolo simples como HTTP e um serviço online (como webhook.site ou similar) que permita receber requisições HTTP POST e exibir os dados.
4. Configuração (Kconfig)
As seguintes opções serão configuráveis via Kconfig:
CONFIG_RADAR_SENSOR_DISTANCE_MM: Distância entre os dois sensores em milímetros (inteiro).
CONFIG_RADAR_SPEED_LIMIT_KMH: Limite de velocidade em km/h (inteiro).
CONFIG_RADAR_CAMERA_FAILURE_RATE_PERCENT: Porcentagem de chance da câmera simular uma falha e retornar uma placa inválida (inteiro 0-100).

5. Estratégia de Testes
Será adotada uma estratégia de testes que combina testes automatizados e manuais:
Testes de Unidade (Automatizados):
Função de cálculo de velocidade (dado tempo e distância, verifica a velocidade calculada).
Função de validação de placa Mercosul (testar diversos formatos válidos e inválidos para BR, AR, PY).
Funções de formatação dos dados para envio à nuvem.
Será utilizada a estrutura de testes ztest do Zephyr para implementar testes unitários em partes do código.


Testes de Integração (Automatizados/Semi-automatizados):
Testar o fluxo completo: Simular eventos de GPIO para os sensores, verificar se a velocidade é calculada e a infração detectada.
Verificar se os dados da infração são corretamente passados para a thread de rede.
Testar a comunicação via ZBUS entre a thread principal e a simulação da câmera.


Testes Manuais:
Execução no QEMU e observação do Display Dummy.
Utilização das ferramentas de simulação de GPIO do QEMU para simular passagens de veículos com diferentes velocidades.
Monitoramento do tráfego de rede (e.g., com Wireshark ou similar, caso seja aplicável) para verificar o envio correto dos pacotes HTTP para o serviço na nuvem.
Verificação dos dados recebidos no serviço web na nuvem.



Será dada ênfase à criação de testes automatizados, alinhado aos critérios de avaliação.
6. Gerenciamento do Projeto e Cronograma
O projeto será desenvolvido utilizando Git para controle de versão, com um repositório compartilhado apenas entre a equipe e o professor (GitHub, login do professor no github: rodrigopex).

Cronograma (sugerido):
Semana 1 (até Entrega Parcial - Dia 22/Maio):
Configuração inicial do ambiente Zephyr e QEMU.
Implementação do cálculo básico de velocidade (assumindo distância fixa inicial).
Configuração básica do Kconfig para distância e limite.
Estrutura básica de threads.
Commit e push da versão parcial.
Configuração e uso inicial do Display Dummy para exibir a velocidade calculada.
Implementação da detecção de infração.
Implementação completa do cliente NTP e obtenção de data/hora.


Semana 2:
Implementação da lógica de validação de placa Mercosul.
Integração da Thread Principal com a câmera simulada via ZBUS.
Desenvolvimento e escrita dos testes unitários e de integração.
Implementação do envio de dados da infração para a nuvem (escolha e integração do serviço web e protocolo HTTP).
Integração final de todas as threads.
Refinamento do código e documentação (README).
Testes completos do sistema.
Revisão do código e preparação para entrega final.
7. Entregáveis
Link para o repositório Git contendo todo o código-fonte do projeto.
Arquivo README.md detalhado no repositório com:
Descrição do projeto.
Instruções passo a passo para configurar o ambiente e executar o projeto no QEMU.
Explicação das opções configuráveis via Kconfig.
Descrição da arquitetura e funcionamento dos diferentes módulos/threads.
Instruções para rodar os testes (se automatizados).
Detalhes sobre o serviço web na nuvem utilizado.


8. Alinhamento com Critérios de Avaliação
A elaboração e execução deste projeto estão diretamente alinhadas aos critérios de avaliação:
Qualidade de Código: Será priorizada a escrita de código limpo, modular, bem comentado e seguindo boas práticas de programação embarcada. O uso de Git facilitará revisões e organização.
Criatividade: A escolha e implementação do serviço de nuvem, e a forma de apresentar as informações no Display Dummy permitirão demonstrar criatividade na resolução dos problemas.
Aplicação de Testes Automáticos: A inclusão de testes unitários e de integração utilizando ztest (ou abordagem similar) será uma parte fundamental do desenvolvimento.
Uso Adequado do Zephyr: O projeto explora diversas funcionalidades do Zephyr RTOS (multithreading, Kconfig, drivers, ZBUS, networking), demonstrando um bom entendimento e aplicação da framework.
Uso Adequado do Git: O uso de branches, commits significativos e um fluxo de trabalho colaborativo (em caso de dupla) garantirá um bom uso da ferramenta.
9. Conclusão
Acreditamos que esta proposta de projeto aborda todos os requisitos da disciplina de forma desafiadora, mas realizável dentro do prazo. A estrutura modular e o uso de Zephyr permitirão um desenvolvimento organizado e o aprendizado prático de conceitos fundamentais de sistemas embarcados. Estamos confiantes em entregar um sistema funcional, bem documentado e com boa qualidade de código.