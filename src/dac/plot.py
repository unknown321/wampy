import plotly.graph_objects as go
from plotly.subplots import make_subplots


def main():
    tables = {}
    title = ""
    with open("/tmp/data", "r") as f:
        lines = f.readlines()
        title = lines[0].split(" ")[1].rstrip("\n")
        for line in lines[1:]:
            parts = line.split(" ")
            if str(parts[0]) not in tables:
                tables[str(parts[0])] = []

            tables[parts[0]].append(int(parts[1].rstrip("\n")))

    cols = 0
    for k, v in tables.items():
        if sum(v) > 0:
            cols += 1

    fig = make_subplots(rows=1, cols=cols, shared_yaxes=True)

    row = 1
    col = 1
    for k, v in tables.items():
        if sum(v) == 0:
            continue

        x = list(range(0, len(v)))
        print(len(x), len(v))
        fig.add_trace(go.Scatter(x=x, y=v, name="table {}".format(k)), row=row, col=col)
        col = col + 1

    # fig.update_layout(autosize=True, height=20000)
    fig.update_xaxes(rangeslider=dict(visible=False))
    fig.update_layout(title_text=title)

    fig.show()


main()
