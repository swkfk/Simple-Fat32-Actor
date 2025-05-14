#define ROUND_UP(val, base) (((val) + (base) - 1) & ~((base) - 1))
#define ROUND_DOWN(val, base) ((val) & ~((base) - 1))

#define IN_RANGE(val, range) (((val) >= range##_LOW) && ((val) < range##_HIGH))
