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

THRESHOLDS = (
    (2, 2, 2, 2, 2, 2),
    (3, 4, 5, 7, 9, 12),
    (4, 5, 7, 9, 12, 17),
    (5, 6, 8, 11, 15, 21),
    (5, 7, 9, 12, 17, 24),
    (6, 7, 10, 14, 19, 26),
    (6, 8, 11, 15, 21, 29),
    (6, 9, 12, 16, 22, 31),
    (7, 9, 12, 17, 24, 33),
    (7, 10, 13, 18, 25, 35),
    (7, 10, 14, 19, 26, 37),
    (8, 10, 14, 20, 28, 39),
    (8, 11, 15, 21, 29, 40),
    (8, 11, 15, 21, 30, 42),
    (9, 12, 16, 22, 31, 43),
    (9, 12, 17, 23, 32, 45),
    (9, 12, 17, 24, 33, 46),
    (9, 13, 18, 24, 34, 48),
    (10, 13, 18, 25, 35, 49),
    (10, 13, 18, 26, 36, 50),
    (10, 14, 19, 26, 37, 52),
    (10, 14, 19, 27, 38, 53),
    (10, 14, 20, 28, 39, 54),
    (11, 15, 20, 28, 39, 55),
    (11, 15, 21, 29, 40, 56),
    (11, 15, 21, 29, 41, 58),
    (11, 15, 21, 30, 42, 59),
    (11, 16, 22, 30, 43, 60),
    (12, 16, 22, 31, 43, 61),
    (12, 16, 23, 31, 44, 62),
    (12, 17, 23, 32, 45, 63),
    (12, 17, 23, 33, 46, 64),
    (13, 17, 24, 34, 47, 66),
    (13, 18, 25, 34, 48, 68),
    (13, 18, 25, 35, 50, 70),
    (14, 19, 26, 36, 51, 72),
    (14, 19, 27, 37, 52, 73),
    (14, 20, 27, 38, 53, 75),
    (14, 20, 28, 39, 55, 77),
    (15, 20, 28, 40, 56, 79),
    (15, 21, 29, 41, 57, 80),
    (15, 21, 30, 41, 58, 82),
    (16, 22, 30, 42, 59, 83),
    (16, 22, 31, 43, 60, 85),
    (16, 22, 31, 44, 61, 86),
    (16, 23, 32, 44, 62, 88),
    (17, 23, 32, 45, 63, 89),
    (17, 23, 33, 46, 65, 91),
    (17, 24, 34, 47, 66, 93),
    (18, 25, 34, 48, 68, 96),
    (18, 25, 35, 50, 70, 98),
    (19, 26, 36, 51, 72, 101),
    (19, 27, 37, 52, 73, 103),
    (20, 27, 38, 53, 75, 106),
    (20, 28, 39, 55, 77, 108),
    (20, 28, 40, 56, 79, 111),
    (21, 29, 41, 57, 80, 113),
    (21, 30, 41, 58, 82, 115),
    (22, 30, 42, 59, 83, 117),
    (22, 31, 43, 60, 85, 120),
    (22, 31, 44, 61, 86, 122),
    (23, 32, 44, 62, 88, 124),
    (23, 32, 45, 63, 89, 126),
    (23, 33, 46, 65, 91, 128),
    (24, 34, 47, 66, 93, 131),
    (25, 34, 48, 68, 96, 135),
    (25, 35, 50, 70, 98, 139),
    (26, 36, 51, 72, 101, 142),
    (27, 37, 52, 73, 103, 146),
    (27, 38, 53, 75, 106, 149),
    (28, 39, 55, 77, 108, 153),
    (28, 40, 56, 79, 111, 156),
    (29, 41, 57, 80, 113, 159),
    (30, 41, 58, 82, 115, 163),
    (30, 42, 59, 83, 117, 166),
    (31, 43, 60, 85, 120, 169),
    (31, 44, 61, 86, 122, 172),
    (32, 44, 62, 88, 124, 175),
    (32, 45, 63, 89, 126, 178),
    (33, 46, 65, 91, 128, 181),
    (34, 47, 66, 93, 131, 185),
    (34, 48, 68, 96, 135, 190),
    (35, 50, 70, 98, 139, 196),
    (36, 51, 72, 101, 142, 201),
    (37, 52, 73, 103, 146, 206),
    (38, 53, 75, 106, 149, 211),
    (39, 55, 77, 108, 153, 216),
    (40, 56, 79, 111, 156, 220),
    (41, 57, 80, 113, 159, 225),
    (41, 58, 82, 115, 163, 230),
    (42, 59, 83, 117, 166, 234),
    (43, 60, 85, 120, 169, 238),
    (44, 61, 86, 122, 172, 243),
    (44, 62, 88, 124, 175, 247),
    (45, 63, 89, 126, 178, 251),
    (46, 65, 91, 128, 181, 255),
    (47, 66, 93, 131, 185, 261),
    (48, 68, 96, 135, 190, 269),
    (50, 70, 98, 139, 196, 276),
    (51, 72, 101, 142, 201, 284),
    (52, 73, 103, 146, 206, 291),
    (53, 75, 106, 149, 211, 298),
    (55, 77, 108, 153, 216, 305),
    (56, 79, 111, 156, 220, 311),
    (57, 80, 113, 159, 225, 318),
    (58, 82, 115, 163, 230, 324),
    (59, 83, 117, 166, 234, 330),
    (60, 85, 120, 169, 238, 337),
    (61, 86, 122, 172, 243, 343),
    (62, 88, 124, 175, 247, 349),
    (63, 89, 126, 178, 251, 354),
    (65, 91, 128, 181, 255, 360),
    (66, 93, 131, 185, 261, 369),
    (68, 96, 135, 190, 269, 380),
    (70, 98, 139, 196, 276, 390),
    (72, 101, 142, 201, 284, 401),
    (73, 103, 146, 206, 291, 411),
    (75, 106, 149, 211, 298, 421),
    (77, 108, 153, 216, 305, 430),
    (79, 111, 156, 220, 311, 440),
    (80, 113, 159, 225, 318, 449),
    (82, 115, 163, 230, 324, 458),
    (83, 117, 166, 234, 330, 467),
    (85, 120, 169, 238, 337, 476),
    (86, 122, 172, 243, 343, 484),
    (88, 124, 175, 247, 349, 493),
    (89, 126, 178, 251, 354, 501),
    (91, 128, 181, 255, 360, 509),
    (93, 131, 185, 261, 369, 521),
    (96, 135, 190, 269, 380, 536),
    (98, 139, 196, 276, 390, 552),
    (101, 142, 201, 284, 401, 566),
    (103, 146, 206, 291, 411, 581),
    (106, 149, 211, 298, 421, 595),
    (108, 153, 216, 305, 430, 608),
    (111, 156, 220, 311, 440, 622),
    (113, 159, 225, 318, 449, 635),
    (115, 163, 230, 324, 458, 647),
    (117, 166, 234, 330, 467, 660),
    (120, 169, 238, 337, 476, 672),
    (122, 172, 243, 343, 484, 684),
    (124, 175, 247, 349, 493, 696),
    (126, 178, 251, 354, 501, 708),
    (128, 181, 255, 360, 509, 719),
    (131, 185, 261, 369, 521, 736),
    (135, 190, 269, 380, 536, 758),
    (139, 196, 276, 390, 552, 780),
    (142, 201, 284, 401, 566, 800),
    (146, 206, 291, 411, 581, 821),
    (149, 211, 298, 421, 595, 840),
    (153, 216, 305, 430, 608, 860),
    (156, 220, 311, 440, 622, 879),
    (159, 225, 318, 449, 635, 897),
    (163, 230, 324, 458, 647, 915),
    (166, 234, 330, 467, 660, 933),
    (169, 238, 337, 476, 672, 950),
    (172, 243, 343, 484, 684, 967),
    (175, 247, 349, 493, 696, 984),
    (178, 251, 354, 501, 708, 1001),
    (181, 255, 360, 509, 719, 1017),
    (185, 261, 369, 521, 736, 1041),
    (190, 269, 380, 536, 758, 1072),
    (196, 276, 390, 552, 780, 1102),
    (201, 284, 401, 566, 800, 1131),
    (206, 291, 411, 581, 821, 1160),
    (211, 298, 421, 595, 840, 1188),
    (216, 305, 430, 608, 860, 1215),
    (220, 311, 440, 622, 879, 1242),
)
WINDOW_EXP = (
    1, 1, 1, 1, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6,
)
MASK32 = 0xFFFFFFFF
pfx = [0] * 64
centers = [0] * 6
widths = [1] * 6
biases = [0] * 6
pn = 0
index = 0
length = 32
qref = 0
rounded_reference = 0
threshold_row_index = 0
output = 0.0


def cst_v8_init():
    global pn, index, length, qref, rounded_reference, threshold_row_index, output
    pn = 0
    index = 0
    length = 32
    qref = 0
    rounded_reference = 0
    threshold_row_index = 0
    output = 0.0
    i = 0
    while i < 64:
        pfx[i] = 0
        i += 1
    i = 0
    while i < 6:
        centers[i] = 0
        widths[i] = 1
        biases[i] = 0
        i += 1


def threshold_index(level):
    if level < 32:
        return level
    shift = 1
    mantissa = level >> 1
    if mantissa >= 4096:
        mantissa >>= 8
        shift += 8
    if mantissa >= 256:
        mantissa >>= 4
        shift += 4
    if mantissa >= 64:
        mantissa >>= 2
        shift += 2
    if mantissa >= 32:
        mantissa >>= 1
        shift += 1
    return (shift << 4) + mantissa


def cst_v8_step(counts):
    global pn, index, length, qref, rounded_reference, threshold_row_index, output
    
    counts = int(counts)
    pn = (pn + counts + 2) & MASK32
    head = index
    old_length = length
    exponent = WINDOW_EXP[old_length]
    full = (pn - pfx[(head + 64 - (1 << exponent)) & 63]) & MASK32
    pfx[head] = pn
    index = (head + 1) & 63

    narrow = (full | qref) < 61440
    hits = 0
    p = exponent
    while p >= 1:
        k = 1 << p
        mask = 0xFFFF if narrow or p <= 2 else MASK32
        if p == exponent:
            total = full & mask
        else:
            total = ((pn & mask) - (pfx[(head + 64 - k) & 63] & mask)) & mask
        if narrow:
            biased = (total - centers[p - 1]) & 0xFFFF
        else:
            expected = qref >> (6 - p)
            biased = (total - expected + biases[p - 1]) & mask
        if biased > widths[p - 1]:
            hits += old_length - (k >> 1) if p == exponent else k >> 1
        p -= 1
    length = old_length + 1 - hits
    if length > 64:
        length = 64
    new_q = full << (6 - exponent)
    if new_q == qref:
        return output
    qref = new_q
    rounded = (new_q + 32) >> 6
    if rounded != rounded_reference:
        rounded_reference = rounded
        row_index = threshold_index(rounded)
        if row_index != threshold_row_index:
            threshold_row_index = row_index
            row = THRESHOLDS[row_index]
            j = 0
            while j < 6:
                biases[j] = row[j] - 2
                widths[j] = (row[j] << 1) - 3
                j += 1
    # These bounds are only consumed when the previous reference fits.
    if new_q < 61440:
        j = 0
        while j < 6:
            centers[j] = ((new_q >> (5 - j)) - biases[j]) & 0xFFFF
            j += 1
    if new_q < 128:
        output = 0.0
    else:
        output = (new_q - 128) / 64.0
    return output


def cst_v8(samples):
    cst_v8_init()
    rates = []
    for sample in samples:
        rates.append(cst_v8_step(sample))
    return rates

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
    if 'b' in plots: plots['b'][0].set_ydata(cst_v8(src_array))
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
    'b': ax.plot(ref_array, linestyle='-', color='#00C000', label = 'cst_v8'),
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
