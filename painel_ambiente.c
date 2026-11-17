/* Painéis de frequência, janela, porta, colchão e objetos narrativos. */
#include "paineis.h"
#include "render.h"
#include "widgets.h"
#include <math.h>

static void desenhar_onda(Rectangle area, float tempo, float amplitude) {
    DrawRectangleRounded(area, 0.04f, 6, (Color){8, 20, 16, 255});
    Vector2 anterior = {area.x, area.y + area.height / 2};
    for (float x = 2; x <= area.width; x += 2) {
        float y = area.y + area.height / 2 + sinf(x * 0.08f + tempo * 5.0f) * amplitude * sinf(x * 0.01f + tempo);
        Vector2 ponto = {area.x + x, y};
        DrawLineEx(anterior, ponto, 2, COLOR_SIGNAL);
        anterior = ponto;
    }
    ui_scanlines(area);
}

void painel_frequencia(GameState *game, Rectangle area) {
    Partida *partida = &game->partida;
    float amplitude = 20.0f + partida->rastreamento * 0.9f; /* quanto mais rastreado, mais o sinal "aparece" */
    desenhar_onda((Rectangle){area.x, area.y, 560, 260}, game->clock, amplitude);
    Rectangle coluna = {area.x + 590, area.y, area.width - 590, area.height};
    ui_medidor((Rectangle){coluna.x, coluna.y, coluna.width, 44}, "SINAL EXPOSTO", partida->rastreamento,
               partida->rastreamento >= 70 ? COLOR_ALERT : COLOR_SIGNAL);
    render_text_wrapped("Cada transmissão deixa um rastro no ar. Se o sinal ficar totalmente exposto, um carro de escuta acha a "
                        "rádio e a noite acaba. Trocar de frequência embaralha o rastro.",
                        (Rectangle){coluna.x, coluna.y + 64, coluna.width, 0}, 15, COLOR_CREAM);
    if (ui_botao_principal((Rectangle){coluna.x, coluna.y + 200, coluna.width, 54}, "Trocar de frequência", 1,
                           partida->fase == FASE_OPERACAO && partida->rastreamento > 0))
        partida_trocar_frequencia(partida);
    if (partida->fase == FASE_PREPARACAO) render_text("Disponível depois de ligar a rádio.", coluna.x, coluna.y + 264, 13, COLOR_MUTED);
}

void painel_janela(GameState *game, Rectangle area) {
    Partida *partida = &game->partida;
    render_text(partida->cortina_fechada ? "A cortina está fechada." : "A cortina está aberta.", area.x, area.y, 22, COLOR_AMBER);
    char hora[6];
    partida_formatar_hora(partida->holofote_chegada, hora);
    const char *situacao = partida->holofote == EMERGENCIA_AVISADA
                               ? TextFormat("A chuva escorre pelo vidro. Às %s o holofote da torre vai varrer esta janela: quem estiver "
                                            "na luz, com a cortina aberta, será visto.", hora)
                           : partida->holofote == EMERGENCIA_RESOLVIDA ? "O holofote já passou por aqui esta noite. Por enquanto, só chuva."
                           : partida->holofote == EMERGENCIA_AGUARDANDO ? "A torre está escura, mas o holofote está lá. Ele vai virar para cá."
                                                                        : "Só a chuva e a rua vazia. Nenhum holofote esta noite.";
    render_text_wrapped(situacao, (Rectangle){area.x, area.y + 44, area.width, 0}, 18, COLOR_CREAM);
    render_text_wrapped("Puxar a cortina leva tempo. A outra saída é simplesmente não estar na luz quando ela passar.",
                        (Rectangle){area.x, area.y + 130, area.width, 0}, 15, COLOR_MUTED);
    const char *rotulo = partida->cortina_fechada ? "Abrir a cortina" : "Fechar a cortina";
    if (ui_botao_principal((Rectangle){area.x, area.y + 190, 340, 54}, rotulo, 1, partida->fase == FASE_OPERACAO))
        partida_alternar_cortina(partida);
}

static void desenhar_checagem(float x, float y, const char *texto, bool ok) {
    render_text(ok ? "OK" : "!!", x, y, 16, ok ? COLOR_SIGNAL : COLOR_ALERT);
    render_text(texto, x + 40, y, 17, ok ? COLOR_CREAM : COLOR_ALERT);
}

void painel_porta(GameState *game, Rectangle area) {
    Partida *partida = &game->partida;
    int faltam = partida_acoes_ate_inspecao(partida);
    if (faltam < 0) {
        const char *texto = partida->inspecao_superada ? "Os passos se afastaram escada abaixo. O corredor voltou ao silêncio."
                                                       : "Pelo olho mágico, só o corredor vazio e a lâmpada piscando.";
        render_text_wrapped(texto, (Rectangle){area.x, area.y, area.width, 0}, 20, COLOR_CREAM);
        return;
    }
    char hora[6];
    partida_formatar_hora(partida->inspecao_chegada, hora);
    render_text(TextFormat("Botas na escada. Eles batem à porta às %s.", hora), area.x, area.y, 22, COLOR_ALERT);
    const char *suspeita = partida->suspeita < 25 ? "baixa" : partida->suspeita < 50 ? "média" : "alta";
    desenhar_checagem(area.x, area.y + 54, "Papéis escondidos (bancada)", partida->provas_escondidas);
    desenhar_checagem(area.x, area.y + 86, "Equipamentos desligados (transmissor)", !partida->equipamentos_ligados);
    desenhar_checagem(area.x, area.y + 118, TextFormat("Desconfiança sobre você: %s", suspeita), partida->suspeita < 40);
    render_text_wrapped("Se atender agora, a revista acontece já. Se esperar, eles entram na hora marcada de qualquer jeito. "
                        "Papel à vista é o fim; um transmissor quente e uma ficha suja pioram tudo.",
                        (Rectangle){area.x, area.y + 170, area.width, 0}, 15, COLOR_MUTED);
    if (ui_botao_principal((Rectangle){area.x, area.y + 250, 340, 54}, "Atender a porta", 0, true)) partida_responder_porta(partida);
}

void painel_colchao(GameState *game, Rectangle area) {
    Partida *partida = &game->partida;
    if (partida->fase != FASE_OPERACAO) {
        render_text_wrapped("Ainda não. A noite nem começou: prepare-se e ligue a rádio no transmissor.",
                            (Rectangle){area.x, area.y, area.width, 0}, 20, COLOR_CREAM);
        return;
    }
    char fim[6];
    partida_formatar_hora(partida->relogio + partida->pontos_acao, fim);
    render_text("Deitar agora encerra a noite.", area.x, area.y, 22, COLOR_CREAM);
    render_text(TextFormat("A bateria ainda aguentaria até %s. Esse tempo se perde.", fim), area.x, area.y + 34, 16, COLOR_MUTED);
    float y = area.y + 76;
    int total;
    const Tarefa *tarefas = campanha_tarefas(&total);
    render_text("AINDA FALTA:", area.x, y, 13, COLOR_AMBER);
    y += 24;
    bool alguma = false;
    for (int i = 0; i < total; i++) {
        if (!tarefas[i].obrigatoria || !partida_tarefa_visivel(partida, &tarefas[i]) || partida_tarefa_concluida(partida, &tarefas[i])) continue;
        render_text(TextFormat("- %s", tarefas[i].texto), area.x + 10, y, 16, COLOR_ALERT);
        y += 26;
        alguma = true;
    }
    if (!alguma) render_text("Nada essencial. Você fez o que precisava.", area.x + 10, y, 16, COLOR_SIGNAL);
    if (ui_botao_principal((Rectangle){area.x, area.y + area.height - 70, 360, 54}, "Apagar a luz e dormir", -1, true))
        partida_dormir(partida);
}

void painel_objeto(GameState *game, Rectangle area) {
    const TextoObjeto *objeto = campanha_objeto(game->objeto_aberto);
    /* O título já está no cabeçalho do painel; aqui só o que se vê. */
    render_text_wrapped(objeto->texto, (Rectangle){area.x, area.y + 10, area.width, 0}, 20, COLOR_CREAM);
    render_text("Olhar não custa tempo.", area.x, area.y + area.height - 24, 14, COLOR_MUTED);
}
