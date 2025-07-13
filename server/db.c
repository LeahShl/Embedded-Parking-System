#include <stdio.h>

void init_db(void);

void add_city(const char *name);

void add_lot(int city_id, int hourly, int price, int md_price);

void remove_lot(int lot_id);

void update_price(int lot_id, int price);

void toggle_hourly(int lot_id);

void update_max_price(int lot_id, int md_price);

void add_log(int lot_id, int customer_id, const char *starttime,
             const char *endtime, double duration, int price);

int main(void) {
    printf("Hello\n");
    return 0;
}