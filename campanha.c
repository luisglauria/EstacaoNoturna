/* Textos e dados das quatro noites. Para ajustar o enredo, mexa só neste arquivo.
   Ids de documento começam em 1 (0 = "nenhum"). Prazos contam ações da noite (cada ação = 15 min,
   a noite começa às 23:00: prazo 5 = até 00:15). */
#include "campanha.h"
#include <stddef.h>

#define CONTAR(vetor) ((int)(sizeof(vetor) / sizeof((vetor)[0])))

/* Ids dos documentos, usados nas contradições e nas tarefas. */
enum { DOC_CADERNO = 1, DOC_MAPA, DOC_CHAMADAS, DOC_ESCALA, DOC_RELATO_PADARIA, DOC_GRAVACAO, DOC_ORDEM };

/* Áreas do holofote em coordenadas da tela virtual (alinhadas à grade de pixels da cabine). */
static const DefinicaoNoite NOITES[TOTAL_NOITES] = {
    {"Primeiro Sinal",
     "O antigo operador desapareceu há uma semana. A rádio agora é sua. Esta noite é tranquila: conheça a "
     "cabine, desconfie das mensagens e faça a primeira transmissão.",
     6, 0, 0, {0, 0, 0, 0}, 0, 0},
    {"Luz na Janela",
     "Instalaram um holofote na torre do outro lado da rua. E duas vozes diferentes juram ter a senha nova. "
     "Só uma delas diz a verdade.",
     8, 5, 2, {840, 102, 291, 300}, 0, 0},
    {"Alguém Está Ouvindo",
     "A polícia anda chegando antes dos avisos. Alguém está passando o roteiro adiante. Descubra quem, antes "
     "que seja tarde demais.",
     8, 4, 2, {147, 102, 303, 300}, 0, 0},
    {"Última Transmissão",
     "Esta noite a denúncia vai ao ar, com as provas que você tiver. Lá embaixo, os agentes já começaram a "
     "revistar o prédio.",
     9, 0, 0, {0, 0, 0, 0}, 6, 3},
};

static const Mensagem MENSAGENS[] = {
    /* ---------- Noite 1: Primeiro Sinal ---------- */
    {.id = 1, .noite = 1, .chegada = -1, .remetente = "Antigo operador",
     .texto = "Bilhete preso no microfone: 'Abra toda noite com a senha ORION. É assim que os nossos sabem que a voz é amiga.'",
     .veracidade = VERACIDADE_VERDADEIRA,
     .confirmacao = "Garça responde em código curto: ORION continua valendo esta semana.",
     .transmissivel = true,
     .transmissao = {.id = 1, .codigo = "ORION", .titulo = "Senha: ORION", .custo = 1, .prazo = 6},
     .resultado = "A senha atravessa a estática. Em algum lugar, alguém aumenta o volume."},
    {.id = 2, .noite = 1, .chegada = -1, .remetente = "Antigo operador",
     .texto = "Rascunho de uma denúncia sobre os arquivos do Ministério. Na margem, a lápis: 'ainda sem os originais'.",
     .veracidade = VERACIDADE_INCOMPLETA,
     .confirmacao = "Garça: 'Sem os documentos originais, vão chamar de boato. Guarde para quando tivermos as provas.'",
     .transmissivel = true,
     .transmissao = {.id = 2, .codigo = "ARQ", .titulo = "Denúncia dos arquivos (sem originais)", .custo = 2, .prazo = 6,
                     .consequencia = MARCA_DENUNCIA_PRECOCE},
     .resultado = "A denúncia saiu sem provas. De manhã, o Ministério negou tudo e dobrou a vigilância."},
    {.id = 3, .noite = 1, .chegada = 0, .remetente = "Lince",
     .texto = "Família escondida no setor leste. Avise: 'rota R-17 livre'. Eles só confiam se ouvirem a senha antes. "
              "Rápido: a patrulha fecha o setor perto da meia-noite.",
     .veracidade = VERACIDADE_VERDADEIRA,
     .confirmacao = "Garça confirma: o Lince é de confiança, e a família existe.",
     .transmissivel = true,
     .transmissao = {.id = 3, .codigo = "R-17", .titulo = "Aviso: rota R-17 livre", .custo = 2, .prazo = 5,
                     .requer = "ORION", .consequencia = MARCA_LINCE_AJUDADO | MARCA_BENEFICIO_AVISO, .urgente = true},
     .resultado = "No setor leste, uma porta se abre no escuro. A família atravessa a R-17 antes da patrulha."},
    {.id = 4, .noite = 1, .chegada = 1, .remetente = "Farol",
     .texto = "Boa noite, operador! Sou o Farol, a Garça me deu esta frequência. Me mande o roteiro de cada noite "
              "e eu aviso o pessoal que não tem rádio. Estamos juntos nessa.",
     .veracidade = VERACIDADE_VERDADEIRA,
     .confirmacao = "Garça hesita: 'Indiquei alguém com esse nome, sim. Faz pouco tempo.'",
     .total_respostas = 2,
     .respostas = {{"Compartilhar o roteiro", 0, MARCA_FAROL_COM_PROGRAMACAO, "Você manda o roteiro da noite para o Farol."},
                   {"Guardar para si", 0, 0, "Você decide esperar antes de confiar no Farol."}}},
    {.id = 5, .noite = 1, .chegada = 2, .remetente = "Vega-7",
     .texto = "URGENTE. ORION foi comprometida. Use a senha VEGA no aviso de hoje ou a família cai numa emboscada.",
     .veracidade = VERACIDADE_FALSA,
     .confirmacao = "Ninguém na rede conhece um 'Vega-7'. A mensagem é falsa.",
     .documento_contradiz = DOC_CADERNO,
     .contradicao = "O caderno do operador diz que VEGA foi descoberta pela polícia em março.",
     .transmissivel = true,
     .transmissao = {.id = 5, .codigo = "VEGA", .titulo = "Senha: VEGA", .custo = 1, .prazo = 6},
     .resultado = "VEGA era justamente a senha que a polícia conhecia. Seu sinal ficou marcado."},

    /* ---------- Noite 2: Luz na Janela ---------- */
    {.id = 6, .noite = 2, .chegada = 0, .remetente = "Central",
     .texto = "COMUNICADO DA CENTRAL: a partir desta noite a senha é VEGA. Repasse a todos os contatos.",
     .veracidade = VERACIDADE_FALSA,
     .confirmacao = "Não existe nenhuma 'Central' na rede. Alguém está imitando a gente.",
     .documento_contradiz = DOC_CADERNO,
     .contradicao = "VEGA é a senha que a polícia descobriu em março. Está no caderno do operador.",
     .transmissivel = true,
     .transmissao = {.id = 6, .codigo = "VEGA", .titulo = "Senha: VEGA", .custo = 1, .prazo = 8},
     .resultado = "A polícia ouviu a própria senha velha no ar e cercou o prédio vizinho."},
    {.id = 7, .noite = 2, .chegada = 0, .remetente = "Garça",
     .texto = "ORION vazou. A senha agora é CORVO. Abra com ela antes de qualquer aviso, operador.",
     .veracidade = VERACIDADE_VERDADEIRA,
     .confirmacao = "Lince confirma: a rede inteira trocou para CORVO.",
     .transmissivel = true,
     .transmissao = {.id = 7, .codigo = "CORVO", .titulo = "Senha: CORVO", .custo = 1, .prazo = 8},
     .resultado = "Os contatos reconhecem CORVO. A noite volta a ser nossa."},
    {.id = 8, .noite = 2, .chegada = 1, .remetente = "Garça",
     .texto = "A padaria da rua 9 vai ser revistada. Avise o pessoal de lá o quanto antes, mas sempre depois da senha.",
     .veracidade = VERACIDADE_VERDADEIRA,
     .confirmacao = "Lince viu viaturas paradas perto da rua 9. É verdade.",
     .transmissivel = true,
     .transmissao = {.id = 8, .codigo = "PADARIA", .titulo = "Aviso: revista na padaria", .custo = 1, .prazo = 4,
                     .requer = "CORVO", .consequencia = MARCA_GARCA_AJUDADA | MARCA_BENEFICIO_CONFIRMACAO, .urgente = true},
     .resultado = "O forno da padaria esfria sem ninguém lá dentro: saíram a tempo. A Garça promete retribuir."},
    {.id = 9, .noite = 2, .chegada = 2, .exige_marca = MARCA_LINCE_AJUDADO, .remetente = "Lince",
     .texto = "A família passou. Em troca, consegui fotografar a lista de presos do Ministério. Leve ao ar.",
     .veracidade = VERACIDADE_VERDADEIRA,
     .confirmacao = "Garça reconhece na lista nomes de amigos desaparecidos.",
     .transmissivel = true,
     .transmissao = {.id = 9, .codigo = "LISTA", .titulo = "Prova: lista de presos", .custo = 2, .prazo = 8,
                     .requer = "CORVO", .consequencia = MARCA_PROVA_LISTA},
     .resultado = "Nome por nome, a lista de presos vai ao ar. A primeira prova contra o regime."},
    {.id = 10, .noite = 2, .chegada = 3, .remetente = "Garça",
     .texto = "Tenho fotos da prisão clandestina da rua 4. Divulgue quando puder, depois da senha.",
     .veracidade = VERACIDADE_VERDADEIRA,
     .confirmacao = "Lince estava lá naquela noite: as fotos são reais.",
     .transmissivel = true,
     .transmissao = {.id = 10, .codigo = "FOTOS", .titulo = "Prova: fotos da rua 4", .custo = 2, .prazo = 8,
                     .requer = "CORVO", .consequencia = MARCA_PROVA_FOTOS | MARCA_BENEFICIO_ESCONDERIJO},
     .resultado = "As fotos da rua 4 vão ao ar. Um vizinho que ouvia oferece um fundo falso no piso para esconder papéis."},
    {.id = 11, .noite = 2, .chegada = 1, .remetente = "Farol",
     .texto = "Operador, e o roteiro de hoje? O pessoal sem rádio está contando comigo.",
     .veracidade = VERACIDADE_VERDADEIRA,
     .confirmacao = "Nenhum contato recebeu aviso do Farol até agora. Também ninguém reclamou.",
     .total_respostas = 2,
     .respostas = {{"Compartilhar o roteiro", 0, MARCA_FAROL_COM_PROGRAMACAO, "Você manda o roteiro da noite para o Farol."},
                   {"Guardar para si", 0, 0, "Você não manda nada ao Farol."}}},

    /* ---------- Noite 3: Alguém Está Ouvindo ---------- */
    {.id = 12, .noite = 3, .chegada = 0, .remetente = "Garça",
     .texto = "Tem alguém entregando nosso roteiro à polícia. Nova senha: LUME. Só eu, o Lince e você sabemos. Abra com ela.",
     .veracidade = VERACIDADE_VERDADEIRA,
     .confirmacao = "Lince confirma: combinaram LUME pessoalmente, longe de qualquer rádio.",
     .transmissivel = true,
     .transmissao = {.id = 12, .codigo = "LUME", .titulo = "Senha: LUME", .custo = 1, .prazo = 8,
                     .consequencia = MARCA_CODIGO_RETIRADO},
     .resultado = "LUME vai ao ar. Quem dependia da senha antiga ficou falando sozinho."},
    {.id = 13, .noite = 3, .chegada = 0, .remetente = "Farol",
     .texto = "Calma, operador, não troque a senha agora, isso só confunde. Continue com CORVO. E me mande o roteiro "
              "de amanhã: preciso preparar o pessoal para a denúncia final.",
     .veracidade = VERACIDADE_FALSA,
     .confirmacao = "Garça nunca viu o Farol. Quem o indicou foi um contato preso no mês passado.",
     .documento_contradiz = DOC_GRAVACAO,
     .contradicao = "A gravação da polícia cita o 'informante F.' e a frequência 104.7 dias antes de você usá-la.",
     .marca_ao_expor = MARCA_FAROL_DESMASCARADO,
     .total_respostas = 3,
     .respostas = {{"Mandar o roteiro verdadeiro", 0, MARCA_FAROL_COM_PROGRAMACAO, "Você manda o roteiro de amanhã ao Farol."},
                   {"Mandar um roteiro falso", 1, MARCA_PROGRAMACAO_FALSA,
                    "Você monta um roteiro com horários e lugares errados e manda ao Farol. Se ele for o informante, "
                    "a polícia vai bater na porta errada."},
                   {"Não responder", 0, 0, "Você deixa o Farol sem resposta."}}},
    {.id = 14, .noite = 3, .chegada = -1, .remetente = "Roteiro de ontem",
     .texto = "Roteiro herdado da noite passada: abrir com a senha CORVO.",
     .veracidade = VERACIDADE_FALSA,
     .confirmacao = "Garça: 'CORVO está nas mãos da polícia. Não use mais.'",
     .documento_contradiz = DOC_RELATO_PADARIA,
     .contradicao = "A polícia chegou à padaria sabendo de coisas que só estavam no roteiro com CORVO. A senha vazou.",
     .transmissivel = true,
     .transmissao = {.id = 14, .codigo = "CORVO", .titulo = "Senha: CORVO", .custo = 1, .prazo = 8},
     .resultado = "Enquanto você repetia CORVO, um carro de escuta triangulava o seu sinal."},
    {.id = 15, .noite = 3, .chegada = 2, .remetente = "Garça",
     .texto = "Consegui as escutas do Ministério: eles grampeiam a cidade inteira. Só ponha no ar depois de LUME.",
     .veracidade = VERACIDADE_VERDADEIRA,
     .confirmacao = "Lince reconhece a voz do ministro na gravação.",
     .transmissivel = true,
     .transmissao = {.id = 15, .codigo = "ESCUTAS", .titulo = "Prova: escutas do Ministério", .custo = 2, .prazo = 7,
                     .requer = "LUME", .consequencia = MARCA_PROVA_ESCUTAS},
     .resultado = "A voz do ministro, gravada às escondidas, ecoa em mil cozinhas. Mais uma prova."},
    {.id = 16, .noite = 3, .chegada = 3, .remetente = "Farol",
     .texto = "URGENTE! Avise todo mundo: reunião geral amanhã na praça central. É importante, não pode faltar ninguém!",
     .veracidade = VERACIDADE_FALSA,
     .confirmacao = "Ninguém marcou reunião nenhuma. É uma armadilha para juntar a rede num lugar só.",
     .documento_contradiz = DOC_GRAVACAO,
     .contradicao = "Na gravação, o 'informante F.' promete reunir a rede quando a polícia pedir.",
     .marca_ao_expor = MARCA_FAROL_DESMASCARADO,
     .transmissivel = true,
     .transmissao = {.id = 16, .codigo = "PRACA", .titulo = "Aviso: reunião na praça", .custo = 1, .prazo = 5, .urgente = true},
     .resultado = "A praça estava cercada. Vários contatos foram levados, e a culpa pesa."},

    /* ---------- Noite 4: Última Transmissão ---------- */
    {.id = 17, .noite = 4, .chegada = 0, .remetente = "Garça",
     .texto = "É a última noite. Abra com LUME, como ontem, ou ninguém vai acreditar no que vem depois.",
     .veracidade = VERACIDADE_VERDADEIRA,
     .confirmacao = "Lince confirma: LUME continua valendo.",
     .transmissivel = true,
     .transmissao = {.id = 17, .codigo = "LUME", .titulo = "Senha: LUME", .custo = 1, .prazo = 9},
     .resultado = "LUME no ar. A cidade inteira parece prender a respiração."},
    {.id = 18, .noite = 4, .chegada = 0, .bloqueia_marca = MARCA_DENUNCIA_PRECOCE, .remetente = "Garça",
     .texto = "Os originais dos arquivos do Ministério chegaram. Ponha no ar antes da denúncia final.",
     .veracidade = VERACIDADE_VERDADEIRA,
     .confirmacao = "Lince conferiu os carimbos: são originais.",
     .transmissivel = true,
     .transmissao = {.id = 18, .codigo = "ARQUIVOS", .titulo = "Prova: arquivos originais", .custo = 2, .prazo = 9,
                     .requer = "LUME", .consequencia = MARCA_PROVA_ARQUIVOS},
     .resultado = "Carimbo por carimbo, os arquivos originais vão ao ar."},
    {.id = 19, .noite = 4, .chegada = 1, .remetente = "Garça",
     .texto = "Chegou a hora. A denúncia final vai ao ar depois da senha e de todas as provas que você conseguir.",
     .veracidade = VERACIDADE_VERDADEIRA,
     .confirmacao = "Lince: 'É agora ou nunca.'",
     .transmissivel = true,
     .transmissao = {.id = 19, .codigo = "DENUNCIA", .titulo = "DENÚNCIA FINAL", .custo = 3, .prazo = 9,
                     .requer = "LUME", .consequencia = MARCA_DENUNCIA_FINAL, .urgente = true},
     .resultado = "Sua voz atravessa a cidade. Tudo o que o regime escondeu agora tem nome."},
    {.id = 20, .noite = 4, .chegada = 2, .bloqueia_marca = MARCA_PROVA_ESCUTAS, .remetente = "Lince",
     .texto = "A Garça me passou uma cópia das escutas. Se não foram ao ar ontem, ainda dá tempo.",
     .veracidade = VERACIDADE_VERDADEIRA,
     .confirmacao = "Garça confirma que entregou a cópia ao Lince.",
     .transmissivel = true,
     .transmissao = {.id = 20, .codigo = "ESCUTAS", .titulo = "Prova: escutas do Ministério", .custo = 2, .prazo = 9,
                     .requer = "LUME", .consequencia = MARCA_PROVA_ESCUTAS},
     .resultado = "A voz do ministro, gravada às escondidas, ecoa em mil cozinhas. Mais uma prova."},
    {.id = 21, .noite = 4, .chegada = 2, .bloqueia_marca = MARCA_PROVA_FOTOS, .remetente = "Lince",
     .texto = "Recuperei as fotos da rua 4. Se ainda não foram ao ar, é a última chance.",
     .veracidade = VERACIDADE_VERDADEIRA,
     .confirmacao = "Garça confirma: são as mesmas fotos da rua 4.",
     .transmissivel = true,
     .transmissao = {.id = 21, .codigo = "FOTOS", .titulo = "Prova: fotos da rua 4", .custo = 2, .prazo = 9,
                     .requer = "LUME", .consequencia = MARCA_PROVA_FOTOS},
     .resultado = "As fotos da rua 4 finalmente vão ao ar. Mais uma prova."},
};

static const Documento DOCUMENTOS[] = {
    {DOC_CADERNO, 1, false, "Caderno do operador",
     "Senhas: VEGA, descoberta pela polícia em março (NUNCA usar). ORION, senha desta semana.\n"
     "Gente de confiança: Lince e Garça.\nFarol: chegou há uma semana, indicado pela Garça. Ainda não testei."},
    {DOC_MAPA, 1, false, "Mapa do setor leste",
     "Rota R-17: o túnel da antiga linha de bonde. A patrulha fecha o setor leste perto da meia-noite. "
     "Depois disso, a rota morre."},
    {DOC_CHAMADAS, 1, true, "Registro de chamadas",
     "Três ligações do Lince, de um orelhão do setor leste, pedindo ajuda para uma família. "
     "Nenhuma ligação de alguém chamado 'Vega-7'."},
    {DOC_ESCALA, 2, false, "Escala da patrulha",
     "A torre varre a fachada com o holofote. Hoje o feixe entra pela janela e cai sobre o lado do TRANSMISSOR. "
     "Amanhã deve ser o lado do RECEPTOR."},
    {DOC_RELATO_PADARIA, 3, false, "Relato da padaria",
     "Garça: 'Os agentes chegaram perguntando por coisas que só estavam no nosso roteiro. Quem recebeu o roteiro "
     "é quem está falando.'"},
    {DOC_GRAVACAO, 3, true, "Gravação da polícia",
     "Fita de dois dias atrás: 'O informante F. confirma: a rádio clandestina transmite na 104.7, e ele reúne a "
     "rede quando nós pedirmos.' Naquela data, só o Farol tinha recebido a frequência."},
    {DOC_ORDEM, 4, false, "Ordem de revista",
     "Ofício carimbado: 'Revista no prédio da rua 7 esta noite. Procurar papéis, transmissores quentes e "
     "moradores nervosos.'"},
};

static const Novidade NOVIDADES[] = {
    {1, 0, 0, "BOLETIM OFICIAL", "Toque de recolher às 22h. Possuir rádio não autorizado é crime contra a Ordem. Denuncie."},
    {1, 0, 0, "BILHETE DO ANTIGO OPERADOR",
     "Se você está lendo isto, eu não voltei. A rádio é sua. Abra sempre com a senha, desconfie de quem tem pressa, "
     "e nunca deixe papel à vista."},
    {2, MARCA_LINCE_AJUDADO, 0, "BOLETIM OFICIAL", "Fugitivos escaparam pelo setor leste. Recompensa a quem souber de algo."},
    {2, 0, MARCA_LINCE_AJUDADO, "BOLETIM OFICIAL", "Família de subversivos detida no setor leste. A Ordem agradece aos vizinhos atentos."},
    {2, 0, 0, "RECADO DA GARÇA", "Instalaram um holofote na torre. Hoje ele vai varrer a nossa janela. Cuidado com a luz."},
    {3, MARCA_GARCA_AJUDADA, 0, "BOLETIM OFICIAL", "Padaria da rua 9 encontrada vazia. Os donos estão sendo procurados."},
    {3, 0, MARCA_GARCA_AJUDADA, "BOLETIM OFICIAL", "Donos da padaria da rua 9 presos durante revista de rotina."},
    {3, 0, 0, "RECADO DA GARÇA", "A polícia anda chegando antes dos nossos avisos. Alguém está ouvindo o que não devia."},
    {4, 0, 0, "BOLETIM OFICIAL", "Operação de limpeza no prédio da rua 7 esta noite. Moradores devem permanecer em casa."},
    {4, 0, 0, "RECADO DA GARÇA", "É hoje. Junte as provas e ponha a denúncia no ar antes que batam à sua porta."},
};

/* Etapas que se repetem em toda noite (ler, preparar, ligar). */
#define TAREFAS_DE_ABERTURA(noite, dica_receptor)                                                                       \
    {noite, true, META_LER_NOVIDADES, NULL, 0, 0, 0, "Ler as novidades no receptor", dica_receptor,                     \
     "Você começou a noite sem saber o que acontecia lá fora.", 0},                                                    \
    {noite, true, META_PREPARATIVO, NULL, 0, 0, 0, "Escolher como se preparar (bancada)",                               \
     "Na bancada, embaixo, escolha um preparativo para a noite.", "", 0},                                              \
    {noite, true, META_INICIAR, NULL, 0, 0, 0, "Ligar a rádio no transmissor", "No transmissor, à direita, clique em LIGAR A RÁDIO.", "", 0}

static const Tarefa TAREFAS[] = {
    TAREFAS_DE_ABERTURA(1, "O receptor fica no canto de cima, à esquerda. Chegue perto e aperte E."),
    {1, true, META_TRANSMITIR, "R-17", 0, 0, 0, "Avisar a família: rota R-17 (depois de ORION)",
     "Receba o sinal do Lince, aceite a mensagem e transmita ORION antes da R-17.",
     "A família não ouviu o aviso. O Lince ficou sem notícias.", 5},
    {1, false, META_DESMASCARAR, NULL, 5, 0, 0, "Desmascarar a mensagem falsa",
     "Na bancada, compare a mensagem suspeita com o caderno do operador.", "", 0},
    {1, false, META_EVITAR, "ARQ", 0, 0, 0, "Guardar a denúncia dos arquivos para depois",
     "Sem os originais, ninguém vai acreditar. Tire-a do roteiro.", "", 0},
    {1, false, META_INVESTIGAR, NULL, DOC_CHAMADAS, 0, 0, "Investigar o registro de chamadas",
     "Na bancada, escolha o documento desconhecido e investigue.", "", 0},
    {1, false, META_RASTREAMENTO_MAXIMO, NULL, 50, 0, 0, "Terminar com o sinal pouco exposto (até 50%)",
     "Troque de frequência no painel se o sinal ficar exposto.", "", 0},

    TAREFAS_DE_ABERTURA(2, "Comece pelo receptor."),
    {2, true, META_DESMASCARAR, NULL, 6, 0, 0, "Descobrir qual senha nova é falsa",
     "Duas mensagens trazem senhas diferentes. Compare-as com o caderno na bancada.",
     "Você não descobriu a senha falsa, e a rede passou a desconfiar da rádio.", 10},
    {2, true, META_TRANSMITIR, "CORVO", 0, 0, 0, "Pôr a nova senha CORVO no ar",
     "Aceite a mensagem da Garça e transmita CORVO antes dos avisos.",
     "Sem a senha nova, os contatos não reconheceram a sua voz.", 5},
    {2, true, META_HOLOFOTE, NULL, 0, 0, 0, "Não ser visto pelo holofote",
     "Feche a cortina na janela ou fique fora da área marcada no chão quando a luz passar.", "", 0},
    {2, false, META_TRANSMITIR, "PADARIA", 0, 0, 0, "Avisar a padaria da rua 9 a tempo",
     "Ordenar pelos prazos ajuda, mas o aviso precisa vir depois de CORVO.", "", 0},
    {2, false, META_TRANSMITIR, "LISTA", 0, MARCA_LINCE_AJUDADO, 0, "Divulgar a lista de presos", "Uma prova do Lince.", "", 0},
    {2, false, META_TRANSMITIR, "FOTOS", 0, 0, 0, "Divulgar as fotos da rua 4", "Uma prova da Garça.", "", 0},

    TAREFAS_DE_ABERTURA(3, "Comece pelo receptor."),
    {3, true, META_DESMASCARAR, NULL, 13, 0, 0, "Descobrir quem está vazando o roteiro",
     "Investigue a gravação na bancada e compare com as mensagens do Farol.", "O informante continua dentro da rede.", 10},
    {3, true, META_TRANSMITIR, "LUME", 0, 0, 0, "Trocar a senha vazada: pôr LUME no ar",
     "Tire CORVO do roteiro e transmita LUME.", "Sem senha nova, o informante continua ouvindo tudo.", 10},
    {3, true, META_HOLOFOTE, NULL, 0, 0, 0, "Não ser visto pelo holofote",
     "Hoje a luz cai do lado do receptor: feche a cortina ou saia da área marcada.", "", 0},
    {3, false, META_MARCA, NULL, (int)MARCA_PROGRAMACAO_FALSA, 0, 0, "Enganar o Farol com um roteiro falso",
     "Responda ao pedido do Farol no receptor.", "", 0},
    {3, false, META_TRANSMITIR, "ESCUTAS", 0, 0, 0, "Divulgar as escutas do Ministério", "Precisa de LUME antes.", "", 0},
    {3, false, META_DESMASCARAR, NULL, 16, 0, 0, "Não cair no aviso falso da praça", "Desconfie de pedidos urgentes do Farol.", "", 0},

    TAREFAS_DE_ABERTURA(4, "Comece pelo receptor."),
    {4, true, META_INSPECAO, NULL, 0, 0, 0, "Passar pela revista",
     "Antes de abrir a porta: esconda as provas (bancada) e desligue os equipamentos (transmissor).", "", 0},
    {4, true, META_TRANSMITIR, "DENUNCIA", 0, 0, 0, "Pôr a denúncia final no ar",
     "Primeiro LUME, depois as provas, por último a denúncia.", "A denúncia final nunca chegou a ir ao ar.", 0},
    {4, false, META_PROVAS, NULL, 3, 0, 0, "Ter três provas ou mais no ar", "Conte também as provas das noites anteriores.", "", 0},
    {4, false, META_TRANSMITIR, "ARQUIVOS", 0, 0, MARCA_DENUNCIA_PRECOCE, "Divulgar os arquivos originais",
     "Uma prova que só existe se a denúncia não saiu cedo demais.", "", 0},
    {4, false, META_PROVAS_ESCONDIDAS, NULL, 0, 0, 0, "Esconder as provas antes da revista", "Na bancada.", "", 0},
};

static const DescricaoPreparativo PREPARATIVOS[PREPARO_TOTAL] = {
    {"Nenhum", ""},
    {"Bateria extra", "Mais 30 minutos de energia nesta noite."},
    {"Cortina reforçada", "A primeira passagem do holofote não te vê, mesmo com a cortina aberta."},
    {"Embaralhador", "Cada transmissão expõe só metade do sinal."},
    {"Esconderijo pronto", "Os papéis já começam escondidos sob o piso."},
};

static const TextoObjeto OBJETOS[OBJETO_TOTAL] = {
    {"Fotografia na parede",
     "O antigo operador e duas pessoas sorrindo em frente à padaria da rua 9. No verso, a lápis: "
     "'G. e L., antes de tudo começar.'"},
    {"Bilhete na gaveta",
     "Um papel dobrado em quatro: 'Três batidas na porta: não abra. Duas: é a Garça. E nunca, nunca deixe papel à vista.'"},
    {"Rádio doméstico",
     "Sintonizado na estação oficial. Uma voz calma repete: 'Cidadão, a Ordem protege você. Denuncie vizinhos suspeitos.'"},
    {"Marcas na parede",
     "Riscos contando os dias. O último foi feito há uma semana: o dia em que o operador sumiu."},
};

static const TextoFinal FINAIS[] = {
    {"", ""},
    {"DENÚNCIA COMPLETA",
     "Ao amanhecer, as rádios oficiais tentam desmentir, mas já é tarde: a cidade inteira ouviu, e as provas falam "
     "por si. Na frequência 104.7, alguém deixa uma única palavra no ar: obrigado."},
    {"DENÚNCIA INCOMPLETA",
     "Parte da verdade saiu, mas faltaram provas. O regime chama tudo de boato, e muita gente prefere acreditar. "
     "A rede sobrevive. A luta continua, mais difícil do que antes."},
    {"CAPTURA",
     "As botas sobem a escada. Arrancam as tábuas, os fios, a antena. Antes do amanhecer, a frequência 104.7 é "
     "só estática."},
};

const DefinicaoNoite *campanha_noite(int numero) {
    if (numero < 1 || numero > TOTAL_NOITES) return &NOITES[0];
    return &NOITES[numero - 1];
}

const Mensagem *campanha_mensagens(int *quantidade) {
    *quantidade = CONTAR(MENSAGENS);
    return MENSAGENS;
}

const Mensagem *campanha_mensagem(int id) {
    for (int i = 0; i < CONTAR(MENSAGENS); i++) {
        if (MENSAGENS[i].id == id) return &MENSAGENS[i];
    }
    return NULL;
}

const Documento *campanha_documentos(int *quantidade) {
    *quantidade = CONTAR(DOCUMENTOS);
    return DOCUMENTOS;
}

const Documento *campanha_documento(int id) {
    for (int i = 0; i < CONTAR(DOCUMENTOS); i++) {
        if (DOCUMENTOS[i].id == id) return &DOCUMENTOS[i];
    }
    return NULL;
}

const Novidade *campanha_novidades(int *quantidade) {
    *quantidade = CONTAR(NOVIDADES);
    return NOVIDADES;
}

const Tarefa *campanha_tarefas(int *quantidade) {
    *quantidade = CONTAR(TAREFAS);
    return TAREFAS;
}

const DescricaoPreparativo *campanha_preparativo(Preparativo preparativo) {
    if (preparativo < 0 || preparativo >= PREPARO_TOTAL) return &PREPARATIVOS[0];
    return &PREPARATIVOS[preparativo];
}

const TextoObjeto *campanha_objeto(ObjetoNarrativo objeto) {
    if (objeto < 0 || objeto >= OBJETO_TOTAL) return &OBJETOS[0];
    return &OBJETOS[objeto];
}

const TextoFinal *campanha_final(Final final) {
    if (final < FINAL_NENHUM || final > FINAL_CAPTURA) return &FINAIS[0];
    return &FINAIS[final];
}
