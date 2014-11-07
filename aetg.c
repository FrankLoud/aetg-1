#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>

int factors=60;
int levels=60;
unsigned long max_pairs=0;

// 32 kB tables
#define HASHTABLE_BUCKETS   (1024*32768)

struct hashtable_entry_t {
  unsigned fl1, fl2;
  struct hashtable_entry_t *next;
};

struct hashtable_t {
    struct hashtable_entry_t table[HASHTABLE_BUCKETS];
    unsigned long *value_counts;
};

struct hashtable_t hashtable;
typedef struct hashtable_entry_t hashtable_entry;

void hashtable_init(unsigned max_val)
{
    int i;

    for (i = 0; i < HASHTABLE_BUCKETS; ++i)
    {
        hashtable.table[i].fl1 = 0;
        hashtable.table[i].fl2 = 0;
        hashtable.table[i].next = NULL;
    }

    hashtable.value_counts = calloc(max_val,sizeof(unsigned long));
}

unsigned inline hash(unsigned fl1, unsigned fl2)
{
    return (fl1*fl2) % HASHTABLE_BUCKETS;
}

int inline compare(unsigned fl1_a, unsigned fl2_a, unsigned fl1_b, unsigned fl2_b)
{
    int i;

    if (((fl1_a == fl1_b && fl2_a == fl2_b) || (fl1_a == fl2_b && fl2_a == fl1_b))
        && fl1_a != fl2_a && fl1_b != fl2_b)
        return 1;

    return 0;
}

void hashtable_insert(unsigned fl1, unsigned fl2)
{
    unsigned h = hash(fl1, fl2);
    hashtable_entry *e = &hashtable.table[h];

    while(1)
    {
        if (compare(fl1, fl2, e->fl1, e->fl2))
            return;

        if(!e->next)
        {
            e->next = malloc(sizeof(hashtable_entry));
            e = e->next;
            e->fl1 = fl1;
            e->fl2 = fl2;
            e->next = NULL;

            hashtable.value_counts[fl1]++;
            hashtable.value_counts[fl2]++;

            return;
        }

        e = e->next;
    }
}

int hashtable_find(unsigned fl1, unsigned fl2)
{
    unsigned h = hash(fl1, fl2);
    hashtable_entry *e = &hashtable.table[h];

    while(1)
    {
        if (compare(fl1, fl2, e->fl1, e->fl2))
        {
          return 1;
        }

        if(!e->next)
        {
          return 0;
        }

        e = e->next;
    }
}

int hashtable_remove(unsigned fl1, unsigned fl2)
{
    unsigned h = hash(fl1, fl2);
    hashtable_entry *e = &hashtable.table[h];
    hashtable_entry *prev = NULL;

    while(1)
    {
        if (compare(fl1, fl2, e->fl1, e->fl2))
        {
            hashtable.value_counts[e->fl1]--;
            hashtable.value_counts[e->fl2]--;
            if(prev == NULL)
            {
                if(e->next == NULL)
                {
                    e->fl1 = 0;
                    e->fl2 = 0;
                }
                else
                {
                    e->fl1 = e->next->fl1;
                    e->fl2 = e->next->fl2;
                    prev = e->next;
                    e->next = e->next->next;
                    free(prev);
                }
            }
            else
            {
                prev->next = e->next;
                free(e);
            }
            return 1;
        }

        if(!e->next)
        {
            return 0;
        }

        prev = e;
        e = e->next;
    }
}

void hashtable_list()
{
    for(int i = 0; i < HASHTABLE_BUCKETS; ++i)
    {
        hashtable_entry *e = &hashtable.table[i];

        do
        {
            if(e->fl1 != e->fl2)
                printf("%d %d\n", e->fl1, e->fl2);
            e = e->next;
        } while (e);
    }
}

void generate_pairs()
{
    printf("Generating pairs      ");
    for(int i = 0; i < factors; ++i)
    {
        for(int j = 0; j < levels; ++j)
            for(int f = i+1; f < factors; ++f)
                for(int l = 0; l < levels; ++l)
                {
                    unsigned fl1 = i * levels + j;
                    unsigned fl2 = f * levels + l;

                    hashtable_insert(fl1, fl2);
                    max_pairs ++;
                }
        printf("\b\b\b\b%3d%%",(int)((float) (i+1) / factors * 100.));
        fflush(stdout);
    }
    printf("\n");
}

unsigned get_factor(unsigned fl)
{
    return fl / levels;
}

unsigned max_uncovered()
{
    unsigned best=0, best_v=0;

    for(int i = 0; i < factors*levels; ++i)
    {
        if(hashtable.value_counts[i] > best)
        {
            best = hashtable.value_counts[i];
            best_v = i;
        }
    }

    return best_v;
}

unsigned n_covered(unsigned *test)
{
    unsigned n = 0;

    for(int i = 0; i < factors; ++i)
        for(int j = i; j < factors; ++j)
            if(hashtable_find(test[i], test[j]))
                n++;

    return n;
}

void remove_covered(unsigned *test)
{
    for(int i = 0; i < factors; ++i)
        for(int j = i; j < factors; ++j)
            hashtable_remove(test[i], test[j]);
}

unsigned *construct_candidate(unsigned max_val)
{
    unsigned *test = malloc(factors * sizeof(unsigned));
    unsigned factor_list[factors];

    factor_list[0] = get_factor(max_val);
    test[0] = max_val;

    for(int j = 1; j < factors; ++j)
    {
        // Randomly select a new factor which hasnt been used yet
        unsigned next_f = test[0];
        bool found = false;

        do
        {
            next_f = rand() % factors;

            found = false;
            for(int k = 0; k < j; ++k)
                if(next_f == factor_list[k])
                    found = true;
        } while(found);

        factor_list[j] = next_f;

        unsigned best_pairs = 0;
        unsigned best_list[levels];
        unsigned best_list_size = 0;


        for(unsigned possible_v = factor_list[j] * levels;
                possible_v < (factor_list[j]+1) * levels; ++possible_v)
        {
            unsigned new_pairs = 0;

            // Count number of pairs the new value would cover
            for(int i = 0; i < j; ++i)
                new_pairs += hashtable_find(test[i], possible_v);

            // If we have found a better candidate, add this to a new list
            if(new_pairs > best_pairs)
            {
                best_list_size = 1;
                best_pairs = new_pairs;
                best_list[0] = possible_v;
            }
            // If this is the same as the best, add to current list
            else if(new_pairs == best_pairs)
            {
                best_list[best_list_size++] = possible_v;
            }
        }

        unsigned choice = rand() % best_list_size;
        test[j] = best_list[choice];
    }

    return test;
}

unsigned *construct_row()
{
    const unsigned M = 50;
    unsigned mx = max_uncovered();

    unsigned *tests[M];
    unsigned covered[M];

    #pragma omp parallel for schedule(guided) shared(hashtable)
    for(int i = 0; i < M; ++i)
    {
        tests[i] = construct_candidate(mx);
        covered[i] = n_covered(tests[i]);
    }

    unsigned max=0, max_n=covered[0];

    for(int i = 0; i < M; ++i)
        if(covered[i] > max_n)
        {
            max = i;
            max_n = covered[i];
        }

    for(int i = 0; i < M; ++i)
        if(i != max)
            free(tests[i]);

    remove_covered(tests[max]);

    return tests[max];
}

unsigned count_uncovered()
{
    int n=0;

    for(int i = 0; i < factors*levels; ++i)
        n += hashtable.value_counts[i];

    return n;
}

int main(int argc, char *argv[])
{
    FILE *f = fopen("aetg.txt","w");

    srand(time(NULL));

    hashtable_init(factors*levels);

    generate_pairs();

    unsigned uncovered;

    while((uncovered=count_uncovered()) > 0)
    {
        unsigned *test = construct_row();

        printf("\b\b\b\b\b\b\b\b%3.4f%%", 100.-((float)count_uncovered()/2)/max_pairs*100.);
        fflush(stdout);

        for(int i = 0; i < factors; ++i)
             fprintf(f, "%d, ", test[i]);
        fprintf(f,"\n");
    }
    printf("\n");
    fclose(f);
}