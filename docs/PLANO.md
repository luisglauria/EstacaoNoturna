# Estação Noturna — plano de implementação

Mescla da especificação de funcionalidades com o protótipo atual. Decisões fechadas em 2026-11-17.
Números (PA, porcentagens) são o ponto de partida e serão ajustados nos testes de jogo.

## Decisões

| Tema | Decisão |
|---|---|
| Entrega | Tudo numa entrega: menu, tutorial, configurações, áudio e as 4 noites |
| Transmissão | Uma por vez: "Transmitir próxima" executa o `head`, remove o nó, gasta o custo e avança o tempo |
| Prazo | Número da ação da noite até a qual a transmissão ainda vale (prazo 3 = até a 3ª ação) |
| Risco | Dois medidores: **Rastreamento** (da noite) e **Suspeita** (da campanha) |
| Rastreamento 100% | A polícia acha o sinal: a operação acaba, a programação restante se perde, a suspeita sobe muito. Não é captura |
| Nomes | Programação em `programacao.c` com os nomes do enunciado; o resto do código mantém os nomes atuais |
| Investigação | Comparar mensagem × documento na bancada é grátis e marca a contradição. Custam 1 PA: pedir confirmação (receptor) e investigar (desbloqueia um documento novo) |
| Holofote | Respostas: fechar a cortina (1 PA) e afastar-se da janela (grátis, sair do cone de luz). Ser visto aumenta a suspeita, sem derrota |
| Traição (Farol) | Respostas: retirar o código comprometido e enviar uma programação falsa |
| Inspeção | Aviso com 3 ações. Esconder provas (1 PA), desligar equipamentos (1 PA), responder à porta (grátis, resolve) |
| Benefícios | Aviso de patrulha antecipado, confirmação grátis, esconderijo fácil (esconder provas por 0 PA) |
| Preparativos | Um por noite: bateria extra (+2 PA), cortina reforçada, embaralhador (rastreamento pela metade), esconderijo pronto |
| Obrigatória não feita | A noite termina mesmo assim; cada obrigatória perdida gera uma consequência escrita no encerramento e a campanha continua |
| Encerrar a noite | Interagir com o colchão ("Dormir"), com confirmação que lista as obrigatórias pendentes; ou PA zerado |
| Barra de tarefas | Resumo compacto no canto; lista completa (OBRIGATÓRIAS e OPCIONAIS) abre com TAB por cima da cabine |
| Configurações | Volumes (geral, música, efeitos), tela cheia, dicas na tela; salvas em `settings.cfg` |
| Áudio | Gerado por código (estática, bipes, alarme, ambiente), sem arquivos externos |
| Textos | Escritos num único arquivo de dados narrativos (`campanha.c`), para revisão do grupo |

## Ações e custos

| Ação | Custo |
|---|---|
| Andar, abrir painéis, reler mensagens e documentos, observar objetos | 0 |
| Comparar mensagem × documento | 0 |
| Aceitar/descartar mensagem, reorganizar, priorizar (Insertion Sort) | 0 |
| Pedir confirmação de uma mensagem | 1 |
| Investigar (obter documento novo) | 1 |
| Transmitir a próxima | custo do nó (1–3) |
| Trocar frequência | 1 |
| Fechar/abrir cortina | 1 |
| Esconder provas / desligar equipamentos | 1 |

O custo aparece no botão antes de confirmar. Só ações com custo avançam o relógio da noite,
e é nesse avanço que chegam mensagens novas e correm os prazos de holofote e inspeção.

## Estruturas de dados

- **Lista duplamente encadeada** (`programacao.c`): a programação da rádio. Cada nó tem id, texto,
  custo, prazo, dependência (código que precisa ter ido ao ar antes) e consequência.
  Funções: `inicializarProgramacao`, `inserirTransmissao`, `buscarTransmissao`, `removerTransmissao`,
  `moverTransmissao`, `exibirProgramacao`, `ordenarPorPrazo` (Insertion Sort estável, move nós),
  `executarProximaTransmissao`, `liberarProgramacao`.
- **Fila FIFO encadeada** (`fila_mensagens.c`): mensagens chegando ao receptor, na ordem de chegada.

Ao executar a próxima transmissão:
1. Se o prazo passou → falha ("chegou tarde").
2. Se a dependência não foi ao ar antes → falha (ex.: rota sem palavra-código; contatos não reconhecem).
3. Se a mensagem é falsa → armadilha: suspeita sobe muito.
4. Senão → consequência positiva (contato protegido, prova obtida, benefício para a próxima noite).
Em todos os casos o rastreamento sobe.

## As quatro noites (rascunho do enredo)

1. **Primeiro Sinal** (6 PA, sem emergência). Tutorial em jogo. O antigo operador deixou o código
   ORION. O contato Lince pede para divulgar a rota de fuga R-17, que só funciona depois do código.
   O Farol se apresenta como aliado e pede uma cópia da programação para "avisar os contatos".
2. **Luz na Janela** (8 PA, holofote). Duas mensagens anunciam o novo código: VEGA ("Central")
   e CORVO (contato Garça). O caderno do operador diz que VEGA foi descoberto pela polícia:
   comparar revela a contradição. Se a R-17 deu certo, Lince manda uma prova e um aviso de patrulha.
3. **Alguém Está Ouvindo** (8 PA, holofote). A polícia sabia de algo que só estava na programação.
   Pistas: uma gravação anterior ao fato, e o Farol cita um horário que ainda não tinha sido divulgado.
   O jogador pode retirar o código que o Farol conhece e/ou mandar a ele uma programação falsa.
4. **Última Transmissão** (9 PA, inspeção). Montar e transmitir a denúncia final (depende das provas
   reunidas) e sobreviver à inspeção. Resultado: denúncia completa, incompleta ou captura.

## Telas

Menu (Jogar, Tutorial, Configurações, Sair) → noite (cabine + painéis + TAB tarefas; ESC pausa)
→ encerramento da noite (consequências, recursos, contatos, pistas) → … → final.

## Testes (`make test`)

- Programação: lista vazia, um nó, inserção, busca, remoção, movimentação, ordenação estável,
  execução da próxima, liberação de memória.
- Fila: vazia, enfileirar, desenfileirar, ordem FIFO, liberação.
- Regras: prazo vencido, dependência faltando, mensagem falsa, sucesso, rastreamento em 100%.
