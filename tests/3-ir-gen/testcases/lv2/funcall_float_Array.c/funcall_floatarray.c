int test(float a[]) {
    return a[3];
}
int main(void) {
    float a[10];
    a[3] = 3.14;
    test(a);
    return 0;
}