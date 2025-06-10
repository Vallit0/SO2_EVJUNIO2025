#include <stdio.h>
#include <string.h>
#include <ulfius.h>
#include <jansson.h>

#define PORT 8080

typedef struct {
    int id;
    char title[50];
    char author[50];
} Book;

Book library[100];
int current_id = 1;


json_t* book_to_json(const Book* book) {
    json_t *json_book = json_object();
    json_object_set_new(json_book, "id", json_integer(book->id));
    json_object_set_new(json_book, "title", json_string(book->title));
    json_object_set_new(json_book, "author", json_string(book->author));
    return json_book;
}

int create_book(const struct _u_request *request, struct _u_response *response, void *user_data){

    json_t *json_body = ulfius_get_json_body_request(request, NULL);
    if (!json_body) {
        ulfius_set_string_body_response(response, 400, "Invalid JSON body");
        return U_CALLBACK_CONTINUE;
    }

    const char* title = json_string_value(json_object_get(json_body, "title"));
    const char* author = json_string_value(json_object_get(json_body, "author"));

    if (title && author) {
        
        Book* new_book = &library[current_id - 1];
        
        new_book->id = current_id;
        strncpy(new_book->title, title, sizeof(new_book->title) - 1);
        strncpy(new_book->author, author, sizeof(new_book->author) - 1);

        current_id++;

        ulfius_set_string_body_response(response, 201, "Book created successfully");
    } else {
        ulfius_set_string_body_response(response, 400, "Missing data");
    }

    json_decref(json_body);
    return U_CALLBACK_CONTINUE;
}


int get_book(const struct _u_request *request, struct _u_response *response, void *user_data){
    const char* id_str = u_map_get(request->map_url, "id");
    if (id_str == NULL) {
        return U_CALLBACK_ERROR;
    }

    int id = atoi(id_str);

    Book *book = NULL;
    for (int i = 0; i < 100; i++) {
        if (library[i].id == id) {
            book = &library[i];
        }
    }

    if (book != NULL) {
        json_t *json_book = book_to_json(book);
        ulfius_set_json_body_response(response, 200, json_book);
        json_decref(json_book);
        return U_CALLBACK_CONTINUE;
    } else {
        ulfius_set_string_body_response(response, 404, "Book not found");
        return U_CALLBACK_CONTINUE;
    }

}

int get_all_books(const struct _u_request *request, struct _u_response *response, void *user_data) {
    json_t *json_books = json_array();

    for (int i = 0; i < 100; i++) {
        if (library[i].id != 0) { 
            json_t *json_book = book_to_json(&library[i]);
            json_array_append_new(json_books, json_book); 
        }
    }

    ulfius_set_json_body_response(response, 200, json_books);
    json_decref(json_books);  
    return U_CALLBACK_CONTINUE;
}


int main(void) {
    struct _u_instance instance;

    if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
        fprintf(stderr, "Error initializing instance\n");
        return 1;
    }
    
    ulfius_add_endpoint_by_val(&instance, "POST", "/books", NULL, 0, &create_book, NULL);
    ulfius_add_endpoint_by_val(&instance, "GET", "/book", "/:id", 0, &get_book, NULL);
    ulfius_add_endpoint_by_val(&instance, "GET", "/books", NULL, 0, &get_all_books, NULL);

    if (ulfius_start_framework(&instance) == U_OK) {
        printf("API running on port %d\n", PORT);
        getchar();
    } else {
        fprintf(stderr, "Error starting framework\n");
    }

    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);

    return 0;
}