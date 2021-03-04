from datetime import datetime, timedelta
from random import shuffle

from utils.IO import log_info


def compute_default_order(cnf):
    return [x + 1 for x in range(0, cnf.nvars)]

### FORCE (Aloul et al.)

def force(cnf, time_limit = 60, order = None):
    clauses = cnf.clauses

    if order is None:
        order = compute_default_order(cnf)
        shuffle(order)

    log_info("[FORCE] Start")
    log_info("--------------------------------")
    
    span = force_compute_span(clauses, order)
    log_info(f"Span: {span}")

    now = datetime.now()

    while datetime.now() - now < timedelta(seconds = time_limit):
        cogs_v = {}
        span_old = span

        for i, clause in enumerate(clauses):
            cogs = force_compute_cog(clause, order)

            for x in clause:
                x = abs(x)
                if x in cogs_v:
                    a,b = cogs_v[x]
                    cogs_v[x] = (a+cogs, b+1)
                else:
                    cogs_v[x] = (cogs, 1)

        tlocs = []
        for key, value in cogs_v.items():
            center, n = value
            tlocs.append((key, center / n))


        tlocs = sorted(tlocs, key = lambda x: x[1])

        order = [x[0] for x in tlocs]

        span = force_compute_span(clauses, order)
        log_info(f"Span: {span}")

        if span_old == span:
            break;

    log_info("--------------------------------")
    log_info("[FORCE] End")
    return (order, span)


def force_compute_cog(clause, order):

    cog = sum([order.index(abs(x)) for x in clause])

    return cog / len(clause)


def force_compute_span(clauses, order):

    span = []
    for clause in clauses:
        lspan = 0
        
        indizes = [order.index(abs(x)) for x in clause]
        lspan = max(indizes) - min(indizes)

        span.append(lspan)

    return sum(span)

def force_triage(cnf, n1 = 32):

    orders = []

    for i in range(0, n1):
        log_info(f"Seeding ({i + 1}/{n1})")
        orders.append(force(cnf, 30))

    while len(orders) > 1:

        orders = sorted(orders, key = lambda x: x[1])
        orders = orders[:int(len(orders) / 2)]

        orders2 = []

        for i, t in enumerate(orders):
            (order, span) = t
            log_info(f"Processing seed {i + 1}/{len(orders)}")
            orders2.append(force(cnf, 15, order))

        orders = orders2

    return orders[0]

def sort_clauses_by_span(clauses, order):
    
    spans = []

    for clause in clauses:
        spans.append((clause, force_compute_span([clause], order) / (len(clause))))

    spans = sorted(spans, key = lambda x : x[1])

    return [x for x, _ in spans]