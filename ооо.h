struct ST {
    int n;
    std::vector<int> a;
    vector<int> t;
    ST(int n, vector<int>& arr) :n(n), a(arr), t(vector<int>(4 * n)){
        build(0,0,n);
    }

    void build(int v, int l, int r) {
        if (l == r - 1) {
            t[v] = a[v];
            return;
        }
        int m = (l + r) / 2;
        build(2*v + 1, l,m);
        build(2*v + 2, m,r);
        t[v] = t[2 * v + 1] + t[2 * v + 2];
    }

    void change(int v, int l, int r, int pos, int val) {
        if (l == r - 1 && l == pos) {
            t[v] = val;
            return;
        }
        if (l > pos || pos >= r) return;
        int m = (l + r) / 2;
        change(2*v + 1, l,m,pos,val);
        change(2*v + 2, m,r,pos,val);
        t[v] = t[2 * v + 1] + t[2 * v + 2];
    }

    int ask(int v, int l, int r, int ask_l, int ask_r) {
        if (ask_l <= l && r <= ask_r) return t[v];
        if (ask_l >= r || ask_r <= l) return 0;
        int m = (l + r) / 2;
        return ask(2 * v + 1, l,m,ask_l, ask_r) + ask(2 * v + 2, m,r,ask_l, ask_r);
    }
};