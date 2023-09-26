/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <math.h>
#include <time.h>
#include "client.h"

GtkWidget *labels[30];
GtkWidget *cards[17];


static void draw_page1(GtkWidget *window);
static void draw_page2(GtkWidget *window);

typedef struct node {
    GtkWidget *entry;
    gchar *text;
    gchar *name;
}Params;

static GtkWidget *window = NULL;
static GtkWidget * vbox = NULL;
static GtkWidget *fixed = NULL;
static GtkWidget *fixed1 = NULL;
static GtkWidget *label_name[7];
static GtkWidget *entry_name[7];
static GtkWidget *player_vbox[6];
static GtkWidget *player_hbox[12];
static GtkWidget * button_start = NULL;
static GtkWidget *table = NULL;
static GtkWidget *pool_vbox = NULL;
static GtkWidget *buttons[5];
static GtkWidget *game_pool_label[5];
static gchar pool_label_name[5][100];
static gchar state_label_name[6][20];
static gchar bet_label_name[6][20];
static gchar player_label_name[6][50];
static gchar points_label_name[6][50];
static Params g_params[7];
static gchar g_room_info[7][100];
static gchar g_command[5000];
static gchar g_cards_path[17][200];
static gchar err[500];
static gboolean is_enter_room = FALSE;


static void gtk_draw_init() {
    for (int i=0; i<7; i++) {
        label_name[i] = entry_name[i] = NULL;
        g_room_info[i][0] = '\0';
    }
    g_command[0] = '\0';
    err[0] = '\0';
    is_enter_room = FALSE;
}

static void update_page2(GtkWidget *window);

gboolean receive_gameinfo(gpointer data) {
    if (is_enter_room) {
        ReceiveGameInfo();
        if (!isLatest) update_page2(window);
    }
    return TRUE;
}

void GtkDrawStart(int argc, char *argv[])
{
    gtk_draw_init();
    gtk_init(&argc, &argv);
    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_window_set_title(GTK_WINDOW(window), "Texas Poker Game");
    gtk_window_resize((GtkWindow*)window, 800, 600);

    g_idle_add(receive_gameinfo, NULL);

    draw_page1(window);
}

static void enter_callback( GtkWidget *widget,
                     GtkWidget *entry )
{
    Params* params = (Params*) entry;
    const gchar *entry_text;
    entry_text = gtk_entry_get_text (GTK_ENTRY (params->entry));
    memcpy((gchar*)params->text, entry_text, strlen(entry_text)+1);
    // printf("Entry contents: %s\n", (gchar*)params->text);
}

static void show_eror(const gchar* message) {
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE,
                                    "%s",
                                    message);

    g_signal_connect (dialog, "response",
                    G_CALLBACK (gtk_widget_destroy), NULL);

    gtk_widget_show (dialog);
}

static void show_info(const gchar* message) {
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_INFO,
                                    GTK_BUTTONS_CLOSE,
                                    "%s",
                                    message);

    g_signal_connect (dialog, "response",
                    G_CALLBACK (gtk_widget_destroy), NULL);

    gtk_widget_show (dialog);
}

static void create_callback(GtkWidget *widget,
                 gpointer   data ) {
    sprintf(g_command, "CREAT %s SEAT %s PLAYER %s BOT %s POINT %s ROUND %s SB %s",
            g_room_info[0], g_room_info[1], g_room_info[2],
            g_room_info[3], g_room_info[4], g_room_info[5],
            g_room_info[6]);
    printf("%s\n", g_command);
    char * response = ReceiveMessageFromServer(g_command);
    // const char *response = "SUCCESS";
    printf("%s\n", response);

    /* Initialize gameinfo object */
    memset(&gameinfo, 0, sizeof(gameinfo));

    if (strstr(response, "SUCCESS") != NULL) {
        draw_page2(window);
        is_enter_room = TRUE;
    } else {
        show_eror(response);
    }
}

static void join_callback(GtkWidget *widget,
                 gpointer   data ) {
    sprintf(g_command, "JOIN %s SEAT %s",
            g_room_info[0], g_room_info[1]);
    printf("%s\n", g_command);
    char * response = ReceiveMessageFromServer(g_command);
    printf("%s\n", response);

    /* Initialize gameinfo object */
    memset(&gameinfo, 0, sizeof(gameinfo));

    if (strstr(response, "SUCCESS") != NULL) {
        draw_page2(window);
        is_enter_room = TRUE;
    } else {
        show_eror(response);
    }
}

static void get_seat_callback(GtkWidget *widget,
                 gpointer   data ) {
    sprintf(g_command, "GET SEAT %s", g_room_info[1]);
    printf("%s\n", g_command);
    char * response = ReceiveMessageFromServer(g_command);
    printf("%s\n", response);
    if (strstr(response, "is at seat") != NULL) {
        show_info(response);
    } else {
        show_eror(response);
    }
}

static void create_room(GtkWidget *widget,
                 gpointer   data ) {
    for (int i=0; i<7; i++) {
        if (label_name[i]!= NULL) gtk_container_remove(GTK_CONTAINER(fixed), label_name[i]);
        if (entry_name[i] != NULL) gtk_container_remove(GTK_CONTAINER(fixed), entry_name[i]);
    }
    if (button_start != NULL) {
        gtk_container_remove(GTK_CONTAINER(fixed), button_start);
        button_start = NULL;
    }

    static gchar label_names[7][100] = {"play name: ", "seat: ", "players: ", "bot: ", "point: ", "round: ", "SB: "}; 
    int s_x = 300;
    int s_y = 100;
    for (int i=0; i<7; i++) {
        label_name[i] = gtk_label_new(label_names[i]);
        entry_name[i] = gtk_entry_new();
        gtk_entry_set_max_length (GTK_ENTRY (entry_name[i]), 30);
        g_params[i].entry = entry_name[i];
        g_params[i].text = g_room_info[i];
        g_signal_connect (G_OBJECT (entry_name[i]), "changed",
                        G_CALLBACK(enter_callback),
                        &g_params[i]);
        gtk_fixed_put(GTK_FIXED(fixed), label_name[i], s_x, s_y+i*40);
        gtk_widget_set_size_request(label_name[i], 80, 30);    
        gtk_fixed_put(GTK_FIXED(fixed), entry_name[i], s_x+85, s_y+i*40);
        gtk_widget_set_size_request(entry_name[i], 100, 30);    
    }
    button_start = gtk_button_new_with_label("create");
    g_signal_connect(G_OBJECT (button_start), "clicked",
              G_CALLBACK (create_callback), NULL);
    gtk_fixed_put(GTK_FIXED(fixed), button_start, 700, 540);
    gtk_widget_set_size_request(button_start, 80, 30);    

    gtk_widget_show_all (window);
}

static void join_room(GtkWidget *widget,
                 gpointer   data ) {
    for (int i=0; i<7; i++) {
        if (label_name[i] != NULL) {
            gtk_container_remove(GTK_CONTAINER(fixed), label_name[i]);
            label_name[i] = NULL;
        }
        if (entry_name[i] != NULL) {
            gtk_container_remove(GTK_CONTAINER(fixed), entry_name[i]);
            entry_name[i] = NULL;
        }
    }
    if (button_start != NULL) {
        gtk_container_remove(GTK_CONTAINER(fixed), button_start);
        button_start = NULL;
    }

    static gchar label_names[2][100] = {"play name: ", "seat: "}; 
    int s_x = 300;
    int s_y = 100;
    for (int i=0; i<2; i++) {
        label_name[i] = gtk_label_new(label_names[i]);
        entry_name[i] = gtk_entry_new();
        gtk_entry_set_max_length (GTK_ENTRY (entry_name[i]), 30);
        g_params[i].entry = entry_name[i];
        g_params[i].text = g_room_info[i];
        g_signal_connect (G_OBJECT (entry_name[i]), "changed",
                        G_CALLBACK(enter_callback),
                        &g_params[i]);
        gtk_fixed_put(GTK_FIXED(fixed), label_name[i], s_x, s_y+i*40);
        gtk_widget_set_size_request(label_name[i], 80, 30);    
        gtk_fixed_put(GTK_FIXED(fixed), entry_name[i], s_x+85, s_y+i*40);
        gtk_widget_set_size_request(entry_name[i], 100, 30);    
    }
    button_start = gtk_button_new_with_label("join");
    g_signal_connect(G_OBJECT (button_start), "clicked",
              G_CALLBACK (join_callback), NULL);
    gtk_fixed_put(GTK_FIXED(fixed), button_start, 700, 540);
    gtk_widget_set_size_request(button_start, 80, 30);    

    gtk_widget_show_all (vbox);
}

static void get_seat(GtkWidget *widget,
              gpointer   data ) {
    for (int i=0; i<7; i++) {
        if (label_name[i] != NULL) {
            gtk_container_remove(GTK_CONTAINER(fixed), label_name[i]);
            label_name[i] = NULL;
        }
        if (entry_name[i] != NULL) {
            gtk_container_remove(GTK_CONTAINER(fixed), entry_name[i]);
            entry_name[i] = NULL;
        }
    }
    if (button_start != NULL) {
        gtk_container_remove(GTK_CONTAINER(fixed), button_start);
        button_start = NULL;
    }

    label_name[1] = gtk_label_new("seat: ");
    entry_name[1] = gtk_entry_new();
    gtk_entry_set_max_length (GTK_ENTRY (entry_name[1]), 30);
    g_params[1].entry = entry_name[1];
    g_params[1].text = g_room_info[1];
    g_signal_connect (G_OBJECT (entry_name[1]), "changed",
                    G_CALLBACK(enter_callback),
                    &g_params[1]);
    int s_x = 300;
    int s_y = 100;
    gtk_fixed_put(GTK_FIXED(fixed), label_name[1], s_x, s_y);
    gtk_widget_set_size_request(label_name[1], 80, 30);
    gtk_fixed_put(GTK_FIXED(fixed), entry_name[1], s_x+85, s_y);
    gtk_widget_set_size_request(entry_name[1], 100, 30);    

    button_start = gtk_button_new_with_label("get");
    g_signal_connect(G_OBJECT (button_start), "clicked",
              G_CALLBACK (get_seat_callback), NULL);
    gtk_fixed_put(GTK_FIXED(fixed), button_start, 700, 540);
    gtk_widget_set_size_request(button_start, 80, 30);    

    gtk_widget_show_all (window);
}

static void draw_page1(GtkWidget *window) {
    GtkWidget *button_create;
    GtkWidget *button_join;
    GtkWidget *button_get;

    vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_add(GTK_CONTAINER(window),vbox);

    fixed = gtk_fixed_new();
    gtk_container_add (GTK_CONTAINER (vbox), fixed);

    button_create = gtk_button_new_with_label("create room");
    button_join = gtk_button_new_with_label("join room");
    button_get = gtk_button_new_with_label("get seat");
    g_signal_connect(G_OBJECT (button_create), "clicked",
              G_CALLBACK (create_room), NULL);
    g_signal_connect(G_OBJECT (button_join), "clicked",
              G_CALLBACK (join_room), NULL);
    g_signal_connect(G_OBJECT (button_get), "clicked",
              G_CALLBACK (get_seat), NULL);

    gtk_fixed_put(GTK_FIXED(fixed), button_create, 10, 15);
    gtk_widget_set_size_request(button_create, 80, 30);

    gtk_fixed_put(GTK_FIXED(fixed), button_join, 90, 15);
    gtk_widget_set_size_request(button_join, 80, 30);    

    gtk_fixed_put(GTK_FIXED(fixed), button_get, 170, 15);
    gtk_widget_set_size_request(button_get, 80, 30);    

    gtk_widget_show_all (window);

    gtk_main ();
}

static void get_image_path(Card *c, char *path) {
    if (c == NULL) {
        strcpy(path, "poker_images/card_back.png"); 
        return;
    }
    int idx = 0;
    switch (c->suit) {
        case DIAMONDS:
            idx = 0;
            break;
        case CLUBS:
            idx += 13;
            break;
        case HEARTS:
            idx += 13*2;
            break;
        case SPADES:
            idx += 13*3;
            break;
        default:
            strcpy(path, "poker_images/card_placeholder.png"); 
    }
    idx += c->rank - 2;
    if (idx < 10)
        sprintf(path, "poker_images/card_0%d.png", idx);
    else
        sprintf(path, "poker_images/card_%d.png", idx);
}

static void operate_callback(GtkWidget *widget,
                      gpointer   data ) {
    const Params* params = (Params*)data;
    // printf("operate %s", params->name);
    if (strcmp(params->name, "BET") == 0) {
        sprintf(g_command, "BET %s", params->text);
    } else if (strcmp(params->name, "Raise:") == 0) {
        sprintf(g_command, "RAISE %s", params->text);
    } else {
        sprintf(g_command, "%s", params->name);
    }
    printf("%s\n", g_command);
    if (!needInput) {
        show_eror("Error: not turn you!");
        return;
    }
    char * response = ReceiveMessageFromServer(g_command);
    printf("%s\n", response);

    if (strstr(response, "FAIL") != NULL) {
        show_eror(response);
    } 
}

static gchar player_state_str[7][20] = {
   "Waiting",
   "Bet",
   "Called",
   "Raised",
   "Checked",
   "Folded",
   "All_in",
};

static void update_page2(GtkWidget *window) {
    int flag[6] = {0, 0, 0, 0, 0, 0};
    // printf("1111\n");
    if (table != NULL) {
        // gtk_container_remove(GTK_CONTAINER(window), table);
        // table = NULL;
        vbox = NULL;
        if (gameinfo.player.seat-1 == gameinfo.smallBlindIndex) {
            sprintf(player_label_name[gameinfo.player.seat-1], "[SB]%s", gameinfo.player.name);
        } else {
            strcpy(player_label_name[gameinfo.player.seat-1], gameinfo.player.name);
        }
        strcpy(state_label_name[gameinfo.player.seat-1], player_state_str[gameinfo.player.state]);
        sprintf(points_label_name[gameinfo.player.seat-1], "P: %d", gameinfo.player.points);
        sprintf(bet_label_name[gameinfo.player.seat-1], "B: %d", gameinfo.player.currentBet);
        flag[gameinfo.player.seat-1] = 1;
        for (int i=0; i<gameinfo.totalPlayers- 1; i++) {
            strcpy(state_label_name[gameinfo.publicPlayers[i].seat-1], player_state_str[gameinfo.publicPlayers[i].state]);
            sprintf(bet_label_name[gameinfo.publicPlayers[i].seat-1], "B: %d", gameinfo.publicPlayers[i].currentBet);
            sprintf(points_label_name[gameinfo.publicPlayers[i].seat-1], "P: %d", gameinfo.publicPlayers[i].points);
            if (gameinfo.publicPlayers[i].seat-1 == gameinfo.smallBlindIndex) {
                sprintf(player_label_name[gameinfo.publicPlayers[i].seat-1], "[SB]%s", gameinfo.publicPlayers[i].name);
            } else {
                strcpy(player_label_name[gameinfo.publicPlayers[i].seat-1], gameinfo.publicPlayers[i].name);
            }
            flag[gameinfo.publicPlayers[i].seat-1] = 1;
        }
        gchar fn[200];
        get_image_path(&gameinfo.player.hand[0], fn);
        strcpy(g_cards_path[(gameinfo.player.seat-1)*2], fn);
        get_image_path(&gameinfo.player.hand[1], fn);
        strcpy(g_cards_path[(gameinfo.player.seat-1)*2+1], fn);
        for (int i=0; i<gameinfo.totalPlayers- 1; i++) {
            get_image_path(NULL, fn);
            strcpy(g_cards_path[(gameinfo.publicPlayers[i].seat-1)*2], fn);
            strcpy(g_cards_path[(gameinfo.publicPlayers[i].seat-1)*2+1], fn);
        }
        for (int i=0; i<6; i++) {
            if (flag[i] == 0) {
                strcpy(state_label_name[i], "None");
                strcpy(player_label_name[i], "no player");
                sprintf(bet_label_name[i], "B: 0");
                sprintf(points_label_name[i], "P: 0");
                strcpy(g_cards_path[i*2],  "poker_images/card_placeholder.png");
                strcpy(g_cards_path[i*2+1],  "poker_images/card_placeholder.png");
            }
        }
        for (int i=12; i<17; i++) {
            get_image_path(&gameinfo.communityCards[i-12], fn);
            strcpy(g_cards_path[i], fn);
        }
        if (gameinfo.stage == 0) {
            strcpy(g_cards_path[12],  "poker_images/card_back.png");
            strcpy(g_cards_path[13],  "poker_images/card_back.png");
            strcpy(g_cards_path[14],  "poker_images/card_back.png");
            strcpy(g_cards_path[15],  "poker_images/card_back.png");
            strcpy(g_cards_path[16],  "poker_images/card_back.png");
        } else if(gameinfo.stage == 1) {
            strcpy(g_cards_path[15],  "poker_images/card_back.png");
            strcpy(g_cards_path[16],  "poker_images/card_back.png");
        } else if (gameinfo.stage == 2) {
            strcpy(g_cards_path[16],  "poker_images/card_back.png");
        }

        sprintf(pool_label_name[0], "pot: %d", gameinfo.pot);
        sprintf(pool_label_name[1], "current bet: %d", gameinfo.currentBet);
        sprintf(pool_label_name[2], "min bet: %d", gameinfo.minBet);
        sprintf(pool_label_name[3], "small blind index: %d", gameinfo.smallBlindIndex);
        sprintf(pool_label_name[4], "total rounds left: %d", gameinfo.totalRoundsLeft);
    } else {
        // printf("1112\n");
        return;
    }
    // printf("1113\n");
    gtk_label_set_label(GTK_LABEL(labels[0]), player_label_name[0]);
    gtk_label_set_label(GTK_LABEL(labels[1]), state_label_name[0]);
    gtk_label_set_label(GTK_LABEL(labels[2]), bet_label_name[0]);
    gtk_label_set_label(GTK_LABEL(labels[3]), points_label_name[0]);
    gtk_image_set_from_file(GTK_IMAGE(cards[0]), g_cards_path[0]);
    gtk_image_set_from_file(GTK_IMAGE(cards[1]), g_cards_path[1]);

    gtk_label_set_label(GTK_LABEL(labels[4]), player_label_name[1]);
    gtk_label_set_label(GTK_LABEL(labels[5]), state_label_name[1]);
    gtk_label_set_label(GTK_LABEL(labels[6]), bet_label_name[1]);
    gtk_label_set_label(GTK_LABEL(labels[7]), points_label_name[1]);
    gtk_image_set_from_file(GTK_IMAGE(cards[2]), g_cards_path[2]);
    gtk_image_set_from_file(GTK_IMAGE(cards[3]), g_cards_path[3]);

    gtk_label_set_label(GTK_LABEL(labels[8]), player_label_name[2]);
    gtk_label_set_label(GTK_LABEL(labels[9]), state_label_name[2]);
    gtk_label_set_label(GTK_LABEL(labels[10]), bet_label_name[2]);
    gtk_label_set_label(GTK_LABEL(labels[11]), points_label_name[2]);
    gtk_image_set_from_file(GTK_IMAGE(cards[4]), g_cards_path[4]);
    gtk_image_set_from_file(GTK_IMAGE(cards[5]), g_cards_path[5]);

    gtk_label_set_label(GTK_LABEL(labels[12]), player_label_name[3]);
    gtk_label_set_label(GTK_LABEL(labels[13]), state_label_name[3]);
    gtk_label_set_label(GTK_LABEL(labels[14]), bet_label_name[3]);
    gtk_label_set_label(GTK_LABEL(labels[15]), points_label_name[3]);
    gtk_image_set_from_file(GTK_IMAGE(cards[6]), g_cards_path[6]);
    gtk_image_set_from_file(GTK_IMAGE(cards[7]), g_cards_path[7]);

    gtk_label_set_label(GTK_LABEL(labels[16]), player_label_name[4]);
    gtk_label_set_label(GTK_LABEL(labels[17]), state_label_name[4]);
    gtk_label_set_label(GTK_LABEL(labels[18]), bet_label_name[4]);
    gtk_label_set_label(GTK_LABEL(labels[19]), points_label_name[4]);
    gtk_image_set_from_file(GTK_IMAGE(cards[8]), g_cards_path[8]);
    gtk_image_set_from_file(GTK_IMAGE(cards[9]), g_cards_path[9]);

    gtk_label_set_label(GTK_LABEL(labels[20]), player_label_name[5]);
    gtk_label_set_label(GTK_LABEL(labels[21]), state_label_name[5]);
    gtk_label_set_label(GTK_LABEL(labels[22]), bet_label_name[5]);
    gtk_label_set_label(GTK_LABEL(labels[23]), points_label_name[5]);
    gtk_image_set_from_file(GTK_IMAGE(cards[10]), g_cards_path[10]);
    gtk_image_set_from_file(GTK_IMAGE(cards[11]), g_cards_path[11]);

    // printf("1114\n");

    int cnt = 0;
    for (int i=0; i<6; i++) {
        if (flag[i] != 0) {
            cnt++;
        }
        if (cnt == gameinfo.currentPlayerIndex+1) {
            gchar player_color[200];
            sprintf(player_color, "<span foreground='red' font_desc='8'>%s</span>", player_label_name[i]);
            gtk_label_set_markup(GTK_LABEL(labels[i*4]), player_color);
        }
    }

    gtk_image_set_from_file(GTK_IMAGE(cards[12]), g_cards_path[12]);
    // printf("1116\n");
    gtk_image_set_from_file(GTK_IMAGE(cards[13]), g_cards_path[13]);
    // static GdkPixbuf *src_1 = gdk_pixbuf_new_from_file(g_cards_path[13], NULL);
    // gtk_image_set_from_pixbuf(GTK_IMAGE(cards[13]), src_1);
    // printf("1117\n");
    gtk_image_set_from_file(GTK_IMAGE(cards[14]), g_cards_path[14]);
    // static GdkPixbuf *src_2 = gdk_pixbuf_new_from_file(g_cards_path[14], NULL);
    // gtk_image_set_from_pixbuf(GTK_IMAGE(cards[14]), src_2);
    // printf("1118\n");
    gtk_image_set_from_file(GTK_IMAGE(cards[15]), g_cards_path[15]);
    // static GdkPixbuf *src_3 = gdk_pixbuf_new_from_file(g_cards_path[15], NULL);
    // gtk_image_set_from_pixbuf(GTK_IMAGE(cards[15]), src_3);
    // printf("1119\n");
    gtk_image_set_from_file(GTK_IMAGE(cards[16]), g_cards_path[16]);
    // printf("1120\n");

    for (int i=0; i<5; i++) {
        gchar color[200];
        sprintf(color, "<span foreground='red' font_desc='10'>%s</span>", pool_label_name[i]);
        gtk_label_set_markup(GTK_LABEL(game_pool_label[i]), color);
        gtk_misc_set_alignment(GTK_MISC(game_pool_label[i]),0,0);
    }

    gtk_widget_show_all (window);
}

static void draw_page2(GtkWidget *window) {
    if (vbox != NULL) {
        gtk_container_remove(GTK_CONTAINER(window), vbox);
        vbox = NULL;
        for (int i=0; i<6; i++) {
            strcpy(state_label_name[i], "None");
            strcpy(bet_label_name[i], "B 0");
            sprintf(player_label_name[i], "player %d", i);
            sprintf(points_label_name[i], "P 0");
        }
        sprintf(pool_label_name[0], "pot: 0");
        sprintf(pool_label_name[1], "current bet: 0");
        sprintf(pool_label_name[2], "min bet: 0");
        sprintf(pool_label_name[3], "small blind index: 0");
        sprintf(pool_label_name[4], "total rounds left: 0");
        for (int i=0; i<17; i++) {
            strcpy(g_cards_path[i], "poker_images/card_placeholder.png");
        }
    }

    //define table
    table = gtk_table_new(8,8, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 10);
    gtk_table_set_col_spacings(GTK_TABLE(table), 10);

    gtk_container_add (GTK_CONTAINER (window), table);

    player_vbox[0] = gtk_vbox_new(FALSE, 5);
    gtk_table_attach_defaults(GTK_TABLE(table), player_vbox[0], 0,2,1,2);
    player_hbox[0] = gtk_hbox_new(FALSE, 5);
    player_hbox[1] = gtk_hbox_new(FALSE, 5);
    gtk_container_add (GTK_CONTAINER(player_vbox[0] ), player_hbox[0]);
    gtk_container_add (GTK_CONTAINER(player_vbox[0] ), player_hbox[1]);
    labels[0] = gtk_label_new(player_label_name[0]);
    labels[1]  = gtk_label_new(state_label_name[0]);
    labels[2] = gtk_label_new(bet_label_name[0]);
    labels[3]  = gtk_label_new(points_label_name[0]);
    labels[24]  = gtk_label_new("Seat 1:");
    gtk_container_add (GTK_CONTAINER(player_hbox[0] ), labels[24]);
    gtk_container_add (GTK_CONTAINER(player_hbox[0] ), labels[0]);
    gtk_container_add (GTK_CONTAINER(player_hbox[1] ), labels[1]);
    gtk_container_add (GTK_CONTAINER(player_hbox[1] ), labels[2]);
    gtk_container_add (GTK_CONTAINER(player_hbox[1] ), labels[3]);
    cards[0] = gtk_image_new_from_file(g_cards_path[0]);
    gtk_table_attach_defaults(GTK_TABLE(table), cards[0], 0,1,0,1);
    cards[1] = gtk_image_new_from_file(g_cards_path[1]);
    gtk_table_attach_defaults(GTK_TABLE(table), cards[1], 1,2,0,1);

    player_vbox[1] = gtk_vbox_new(FALSE, 5);
    gtk_table_attach_defaults(GTK_TABLE(table), player_vbox[1], 3,5,1,2);
    player_hbox[2] = gtk_hbox_new(FALSE, 5);
    player_hbox[3] = gtk_hbox_new(FALSE, 5);
    gtk_container_add (GTK_CONTAINER(player_vbox[1] ), player_hbox[2]);
    gtk_container_add (GTK_CONTAINER(player_vbox[1] ), player_hbox[3]);
    labels[4] = gtk_label_new(player_label_name[1]);
    labels[5]  = gtk_label_new(state_label_name[1]);
    labels[6] = gtk_label_new(bet_label_name[1]);
    labels[7]  = gtk_label_new(points_label_name[1]);
    labels[25]  = gtk_label_new("Seat 2:");
    gtk_container_add (GTK_CONTAINER(player_hbox[2] ), labels[25]);
    gtk_container_add (GTK_CONTAINER(player_hbox[2] ), labels[4]);
    gtk_container_add (GTK_CONTAINER(player_hbox[3] ), labels[5]);
    gtk_container_add (GTK_CONTAINER(player_hbox[3] ), labels[6]);
    gtk_container_add (GTK_CONTAINER(player_hbox[3] ), labels[7]);
    cards[2] = gtk_image_new_from_file(g_cards_path[2]);
    gtk_table_attach_defaults(GTK_TABLE(table), cards[2], 3,4,0,1);
    cards[3] = gtk_image_new_from_file(g_cards_path[3]);
    gtk_table_attach_defaults(GTK_TABLE(table), cards[3], 4,5,0,1);

    player_vbox[2] = gtk_vbox_new(FALSE, 5);
    gtk_table_attach_defaults(GTK_TABLE(table), player_vbox[2], 6,8,1,2);
    player_hbox[4] = gtk_hbox_new(FALSE, 5);
    player_hbox[5] = gtk_hbox_new(FALSE, 5);
    gtk_container_add (GTK_CONTAINER(player_vbox[2] ), player_hbox[4]);
    gtk_container_add (GTK_CONTAINER(player_vbox[2] ), player_hbox[5]);
    labels[8] = gtk_label_new(player_label_name[2]);
    labels[9]  = gtk_label_new(state_label_name[2]);
    labels[10] = gtk_label_new(bet_label_name[2]);
    labels[11]  = gtk_label_new(points_label_name[2]);
    labels[26]  = gtk_label_new("Seat 3:");
    gtk_container_add (GTK_CONTAINER(player_hbox[4] ), labels[26]);
    gtk_container_add (GTK_CONTAINER(player_hbox[4] ), labels[8]);
    gtk_container_add (GTK_CONTAINER(player_hbox[5] ), labels[9]);
    gtk_container_add (GTK_CONTAINER(player_hbox[5] ), labels[10]);
    gtk_container_add (GTK_CONTAINER(player_hbox[5] ), labels[11]);
    cards[4] = gtk_image_new_from_file(g_cards_path[4]);
    gtk_table_attach_defaults(GTK_TABLE(table), cards[4], 6,7,0,1);
    cards[5] = gtk_image_new_from_file(g_cards_path[5]);
    gtk_table_attach_defaults(GTK_TABLE(table), cards[5], 7,8,0,1);

    player_vbox[3] = gtk_vbox_new(FALSE, 5);
    gtk_table_attach_defaults(GTK_TABLE(table), player_vbox[3], 0,2,6,7);
    player_hbox[6] = gtk_hbox_new(FALSE, 5);
    player_hbox[7] = gtk_hbox_new(FALSE, 5);
    gtk_container_add (GTK_CONTAINER(player_vbox[3] ), player_hbox[6]);
    gtk_container_add (GTK_CONTAINER(player_vbox[3] ), player_hbox[7]);
    labels[12] = gtk_label_new(player_label_name[3]);
    labels[13]  = gtk_label_new(state_label_name[3]);
    labels[14] = gtk_label_new(bet_label_name[3]);
    labels[15]  = gtk_label_new(points_label_name[3]);
    labels[27]  = gtk_label_new("Seat 4:");
    gtk_container_add (GTK_CONTAINER(player_hbox[6] ), labels[27]);
    gtk_container_add (GTK_CONTAINER(player_hbox[6] ), labels[12]);
    gtk_container_add (GTK_CONTAINER(player_hbox[7] ), labels[13]);
    gtk_container_add (GTK_CONTAINER(player_hbox[7] ), labels[14]);
    gtk_container_add (GTK_CONTAINER(player_hbox[7] ), labels[15]);
    cards[6] = gtk_image_new_from_file(g_cards_path[6]);
    gtk_table_attach_defaults(GTK_TABLE(table), cards[6], 0,1,7,8);
    cards[7] = gtk_image_new_from_file(g_cards_path[7]);
    gtk_table_attach_defaults(GTK_TABLE(table), cards[7], 1,2,7,8);

    player_vbox[4] = gtk_vbox_new(FALSE, 5);
    gtk_table_attach_defaults(GTK_TABLE(table), player_vbox[4], 3,5,6,7);
    player_hbox[8] = gtk_hbox_new(FALSE, 5);
    player_hbox[9] = gtk_hbox_new(FALSE, 5);
    gtk_container_add (GTK_CONTAINER(player_vbox[4] ), player_hbox[8]);
    gtk_container_add (GTK_CONTAINER(player_vbox[4] ), player_hbox[9]);
    labels[16] = gtk_label_new(player_label_name[4]);
    labels[17]  = gtk_label_new(state_label_name[4]);
    labels[18] = gtk_label_new(bet_label_name[4]);
    labels[19]  = gtk_label_new(points_label_name[4]);
    labels[28]  = gtk_label_new("Seat 5:");
    gtk_container_add (GTK_CONTAINER(player_hbox[8] ), labels[28]);
    gtk_container_add (GTK_CONTAINER(player_hbox[8] ), labels[16]);
    gtk_container_add (GTK_CONTAINER(player_hbox[9] ), labels[17]);
    gtk_container_add (GTK_CONTAINER(player_hbox[9] ), labels[18]);
    gtk_container_add (GTK_CONTAINER(player_hbox[9] ), labels[19]);
    cards[8] = gtk_image_new_from_file(g_cards_path[8]);
    gtk_table_attach_defaults(GTK_TABLE(table), cards[8], 3,4,7,8);
    cards[9] = gtk_image_new_from_file(g_cards_path[9]);
    gtk_table_attach_defaults(GTK_TABLE(table), cards[9], 4,5,7,8);

    player_vbox[5] = gtk_vbox_new(FALSE, 5);
    gtk_table_attach_defaults(GTK_TABLE(table), player_vbox[5], 6,8,6,7);
    player_hbox[10] = gtk_hbox_new(FALSE, 5);
    player_hbox[11] = gtk_hbox_new(FALSE, 5);
    gtk_container_add (GTK_CONTAINER(player_vbox[5] ), player_hbox[10]);
    gtk_container_add (GTK_CONTAINER(player_vbox[5] ), player_hbox[11]);
    labels[20] = gtk_label_new(player_label_name[5]);
    labels[21]  = gtk_label_new(state_label_name[5]);
    labels[22] = gtk_label_new(bet_label_name[5]);
    labels[23]  = gtk_label_new(points_label_name[5]);
    labels[29]  = gtk_label_new("Seat 6:");
    gtk_container_add (GTK_CONTAINER(player_hbox[10] ), labels[29]);
    gtk_container_add (GTK_CONTAINER(player_hbox[10] ), labels[20]);
    gtk_container_add (GTK_CONTAINER(player_hbox[11] ), labels[21]);
    gtk_container_add (GTK_CONTAINER(player_hbox[11] ), labels[22]);
    gtk_container_add (GTK_CONTAINER(player_hbox[11] ), labels[23]);
    cards[10] = gtk_image_new_from_file(g_cards_path[10]);
    gtk_table_attach_defaults(GTK_TABLE(table), cards[10], 6,7,7,8);
    cards[11] = gtk_image_new_from_file(g_cards_path[11]);
    gtk_table_attach_defaults(GTK_TABLE(table), cards[11], 7,8,7,8);

    cards[12] = gtk_image_new_from_file(g_cards_path[12]);
    gtk_table_attach_defaults(GTK_TABLE(table), cards[12], 1,2,3,4);
    cards[13] = gtk_image_new_from_file(g_cards_path[13]);
    gtk_table_attach_defaults(GTK_TABLE(table), cards[13], 2,3,3,4);
    cards[14] = gtk_image_new_from_file(g_cards_path[14]);
    gtk_table_attach_defaults(GTK_TABLE(table), cards[14], 3,4,3,4);
    cards[15] = gtk_image_new_from_file(g_cards_path[15]);
    gtk_table_attach_defaults(GTK_TABLE(table), cards[15], 4,5,3,4);
    cards[16] = gtk_image_new_from_file(g_cards_path[16]);
    gtk_table_attach_defaults(GTK_TABLE(table), cards[16], 5,6,3,4);

    pool_vbox = gtk_vbox_new(FALSE, 5);
    gtk_table_attach_defaults(GTK_TABLE(table), pool_vbox, 7,8,2,5);
    for (int i=0; i<5; i++) {
        game_pool_label[i] = gtk_label_new(pool_label_name[i]);
        gtk_misc_set_alignment(GTK_MISC(game_pool_label[i]),0,0);
        gtk_container_add (GTK_CONTAINER(pool_vbox), game_pool_label[i]);
    }

    fixed1 = gtk_fixed_new();
    gtk_table_attach_defaults(GTK_TABLE(table), fixed1, 4,8,5,6);

    static gchar names[6][20] = {"BET", "CALL", "RAISE", "FOLD", "CHECK", "AllIN"};
    static Params params[6];
    int s_x = 0;
    int s_y = 0;
    for (int i=0; i<6; i++) {
        params[i].name = names[i];
        buttons[i] = gtk_button_new_with_label(names[i]);
        g_signal_connect(G_OBJECT (buttons[i]), "clicked",
                G_CALLBACK (operate_callback), &params[i]);
        gtk_fixed_put(GTK_FIXED(fixed1), buttons[i], s_x, s_y);
        s_x += 50;
        gtk_widget_set_size_request(label_name[i], 45, 35);    
        if (i==0) {
            GtkWidget *bet_entry = gtk_entry_new();
            static gchar bet_count[10];
            gtk_entry_set_max_length (GTK_ENTRY (bet_entry), 5);
            params[i].entry = bet_entry;
            params[i].text = bet_count;
            g_signal_connect (G_OBJECT (bet_entry), "changed",
                            G_CALLBACK(enter_callback),
                            &params[i]);
            gtk_fixed_put(GTK_FIXED(fixed1), bet_entry, s_x, s_y);
            s_x += 50;
            gtk_widget_set_size_request(bet_entry, 45, 30);    
        } else if (i==2) {
            GtkWidget *raise_entry = gtk_entry_new();
            static gchar raise_count[10];
            gtk_entry_set_max_length (GTK_ENTRY (raise_entry), 5);
            params[i].entry = raise_entry;
            params[i].text = raise_count;
            g_signal_connect (G_OBJECT (raise_entry), "changed",
                            G_CALLBACK(enter_callback),
                            &params[i]);
            gtk_fixed_put(GTK_FIXED(fixed1), raise_entry, s_x, s_y);
            s_x += 50;
            gtk_widget_set_size_request(raise_entry, 45, 30);    
        }
    }

    // gtk_widget_show (table);
    gtk_widget_show_all (window);

    // gtk_main();
}