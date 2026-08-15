import math, random, sys
import numpy as np
import matplotlib.pyplot as plt

# Centered significance test
# DOI: 10.1109/tns.2016.2601785
def cst(array):
    max_window_size = 64

    output = []
    event_buf = np.zeros(max_window_size, dtype=int)
    buf_head = 0
    lambda_cur = 0
    lambda_prev = 0
    sigma_cur = 0
    sigma_prev = 0

    l_t = int(max_window_size / 2)
    t_a = 1.0

    for idx, sample in enumerate(array):
        L = 0
        event_buf[buf_head] = sample
        buf_idx = buf_head
        event_sum = sample

        for i in range(1, l_t):
            buf_idx -= 1
            if buf_idx < 0:
                buf_idx = max_window_size - 1

            event_sum += event_buf[buf_idx]
            lambda_cur = event_sum / (i+1)
            sigma_cur = lambda_cur / (i+1)

            if ((lambda_cur - lambda_prev)**2) > (t_a**2 * (sigma_cur + sigma_prev)):
                L += 1

        l_t = l_t - L + 1
        lambda_prev = lambda_cur
        sigma_prev = sigma_cur
        buf_head += 1
        if buf_head >= max_window_size:
            buf_head = 0

        output.append(lambda_cur)

    return output

def cst_mcu(array):
    t_a = 0x0100 # for n in $(seq 0 4095); do printf "0x%04X " $n; echo "sqrt($n/256)" | bc -l; done
    size_max = 64 # keep this value at 64 or lower

    beta = 2

    event_buf = np.zeros(size_max, dtype=int)
    event_idx = 0 # starting value doesn't matter, safe to skip initialization
    output = []

    event_sum_curr = 0 # min: 0, max: 8191
    size_curr = size_max // 2
    for event_sum_next in array:
        event_sum_next += beta
        size_next = size_curr + 1
        event_buf[event_idx % size_max] = event_sum_next
        esn = event_sum_next - event_sum_curr
        esp = event_sum_next + event_sum_curr
        for n in range(event_idx - 1, event_idx - size_curr, -1):
            event_sum_next = event_buf[n % size_max]
            esn += event_sum_next - event_sum_curr # min: -8191*64, max: 8191*64
            esp += event_sum_next + event_sum_curr # min: 0, max: (8191+8191)*64
            size_next -= esn**2 > ((t_a * esp) >> 8)
        output += [event_sum_curr + esn / size_curr - beta]
        event_sum_curr += esn // size_curr
        size_curr = min(size_next, size_max)
        event_idx += 1

    return output

'''
Each window ends at the current second, so a short window's samples
are a subset of a long one's. For Poisson counts their tests then
correlate as sqrt(j/k) -- just the ratio, not the lengths. So 2 vs 4, 
32 vs 64 etc are redundant. 40 and 41 correlate 0.988.
We can space the window sizes by ratio: log2(64/2) = 5, plus another
one at low end for precision, thus 6 tests is enough instead of 63:
2, 3, 4, 8, 16, 32
'''

M     = 64                  # max window, power of 2
BETA  = 2                   # virtual counts, keeps thresholds off zero
MAXIN = 8191                # 13-bit counter
 
grid = [2, 3, 4, 8, 16, 32]
sqk  = [362, 443, 512, 724, 1024, 1448]    # round(256*sqrt(k)) per grid point
 
tsmall = [0] * 256          # exact threshold for m < 256
isqrt8_lo = [0] * 128       # isqrt8 answers for lam < 128, where a Newton seed is worst
isqrt8_seed = [0] * 126     # Newton seeds for lam >= 128, indexed by lam >> 6
 
pfx = [0] * M               # prefix-sum ring
pn = 0
idx = 0
lam = 0                     # level estimate (+BETA)
size = 0                    # current window
lam_ref = 0                 # level at last threshold rebuild
thr = [0] * 6
 
 
def cst_init():
    global pn, idx, lam, size, lam_ref
 
    # tsmall[m]: smallest T with T*(T-1) > 2m
    t = 2
    p = 2
    for m in range(256):
        while p <= 2 * m:
            p += 2 * t                  # (T+1)*T = T*(T-1) + 2T
            t += 1
        tsmall[m] = t
 
    r = 0
    s = 1
    for l in range(128):                
        while s <= 8 * l:
            r += 1
            s += 2 * r + 1              # (r+1)^2 -> (r+2)^2
        isqrt8_lo[l] = r
    for i in range(126):                # midpoint of each 64-level block
        while s <= 8 * (64 * (i + 2) + 32):
            r += 1
            s += 2 * r + 1
        isqrt8_seed[i] = r
 
    for i in range(M):
        pfx[i] = 0
    pn = 0
    idx = 0
    lam = 0
    size = M // 2
    lam_ref = 0
    for j in range(6):
        thr[j] = tsmall[0]
 
 
def isqrt8(lam):
    if lam < 128:
        return isqrt8_lo[lam]
    if lam >= 8192:
        return 255                      
    v = 8 * lam                         # 8*8191 (max cps) fits 16 bits
    r = isqrt8_seed[(lam >> 6) - 2]     # ~6 good bits
    r = (r + v // r) >> 1               # one Newton step -> ~12 bits
    if r * r > v:                       # at most one correction
        r -= 1
    return r
 
 
def cst_refresh():
    # rebuild grid thresholds; only when lam moved > 1/16
    global lam_ref
    lam_ref = lam
    g = isqrt8(lam) # sqrt(8*lam)
    for j in range(6):
        m = grid[j] * lam
        if m < 256:
            thr[j] = tsmall[m]
        else:
            # T ~ (1+sqrt(1+8m))/2, sqrt(8m) = sqrt(8*lam)*sqrt(k)
            # k is independent of cps, so the lookup table can be used: sqk[j] = 256*sqrt(k)
            thr[j] = (((g * sqk[j]) >> 8) + 3) >> 1
 
 
def cst_step(counts):
    global pn, idx, lam, size
 
    x = counts
    if x > MAXIN:
        x = MAXIN
    pn += x + BETA
 
    # window sum in O(1): two prefix sums
    sfull = pn - pfx[(idx - size) & (M - 1)]
    m = size * lam
    esn = sfull - m
 
    # grid tests; each point stands for the lengths since the previous
    cnt = 0
    prev = 1
    for j in range(6):
        k = grid[j]
        if k >= size:
            break
        dev = (pn - pfx[(idx - k) & (M - 1)]) - k * lam
        if dev >= thr[j] or dev <= -(thr[j] - 1):
            cnt += k - prev
        prev = k
 
    # longest window: exact test, esn^2 > esn + 2m with squares cancelled
    if esn * (esn - 1) > 2 * m:
        cnt += size - prev
 
    den = size
    size = size + 1 - cnt               # shrink by hits, grow by 1
    if size > M:
        size = M
 
    rem = esn
    if esn < 0 or esn >= den:           # else mean moved < 1 count: skip
        lam = sfull // den
        rem = sfull - lam * den
        d = lam - lam_ref
        if d < 0:
            d = -d
        if d > (lam_ref >> 4) + 1:
            cst_refresh()
 
    pfx[idx] = pn                       # deferred: k=64 read this slot above
    idx = (idx + 1) & (M - 1)
 
    if lam < BETA:
        return 0.0
    return (lam - BETA) + rem / den
    
def cst_v6(samples):
    cst_init()
    return [cst_step(s) for s in samples]

def dump(array):
    for i, n in enumerate(array):
        print('%04X%c' % (n, ' ' if (i + 1) % 8 else '\n'), end = '')

def refresh():
    seed = int(np.random.rand() * (2**32 - 1))
    #seed = 1628747943
    np.random.seed(seed)
    print(seed)
    plt.title('Filter comparison (seed = %d)' % (seed))

    # Generate synthetic radiation pulse data
    # DOI: 10.1109/LASCAS.2016.7451002
    trials = 10000 # Bernoulli trials per element (1 second)
    src_array = np.zeros(len(ref_array), dtype=int)
    norm_array = ref_array / trials
    for j in range(trials):
        src_array += np.random.random(len(ref_array)) < norm_array

    #dump(src_array)
    #print('---')
    #dump(np.int32(cst_mcu(src_array)))

    if 'a' in plots: plots['a'][0].set_ydata(src_array)
    if 'b' in plots: plots['b'][0].set_ydata(cst_v6(src_array))
    if 'c' in plots: plots['c'][0].set_ydata(cst(src_array))
    if 'd' in plots: plots['d'][0].set_ydata(cst_mcu(src_array))

    fig.canvas.draw()
    fig.canvas.flush_events()

def on_press(event):
    if event.key == ' ':
        refresh()

# Define the reference functions
ref_array = np.concatenate((
    np.full(120, 10),
    np.full(120, 5),
    np.full(120, 100),
    np.fromfunction(lambda i: 60 - 0.5 * i, (120,)),
    np.fromfunction(lambda i: 300/(np.sqrt(2 * np.pi)) * np.exp(-((i - 60)/15)**2 / 2), (120,)),
    np.fromfunction(lambda i: 300/(np.sqrt(2 * np.pi)) * np.exp(-((i - 60)/15)**2 / 2), (120,)),
    np.full(120, 0.33),
    np.full(120, 0.66),
    np.full(120, 0.99),
    np.fromfunction(lambda i: 20/(np.sqrt(2 * np.pi)) * np.exp(-((i - 60)/15)**2 / 2) + 0.99, (120,)),
    np.full(120, 0.5),
    np.full(120, 0.3),
    np.full(360, 0.1),
    np.fromfunction(lambda i: 2/(np.sqrt(2 * np.pi)) * np.exp(-((i - 180)/75)**2 / 2) + 0.1, (360,)),
    np.fromfunction(lambda i: 2/(np.sqrt(2 * np.pi)) * np.exp(-((i - 180)/75)**2 / 2) + 0.1, (360,)),
    np.fromfunction(lambda i: 2/(np.sqrt(2 * np.pi)) * np.exp(-((i - 180)/75)**2 / 2) + 0.1, (360,)),
))

#ref_array += 8000

# Plot data
fig, ax = plt.subplots()
fig.canvas.mpl_connect('key_press_event', on_press)
plots = {
    'a': ax.plot(ref_array, linestyle='-', color='#C0C0C0', label = 'Raw data'),
    '.': ax.plot(ref_array, linestyle='-', color='#000000', label = 'Reference'),
    'b': ax.plot(ref_array, linestyle='-', color='#00C000', label = 'cst_v6'),
    #'c': ax.plot(ref_array, linestyle='-', color='#FF8000', label = 'CST'),
    'd': ax.plot(ref_array, linestyle='-', color='#FF0000', label = 'CST_MCU'),
}

mul = 2
xt = plt.gca().get_xticks()
xt = np.arange(0, len(ref_array), (xt[1] - xt[0] if len(xt) > 1 else 200) / mul)
plt.xticks(xt, xt * mul)

refresh()

plt.xlabel('Seconds')
plt.ylabel('CPS')
plt.grid(True)
plt.legend()
plt.show()
