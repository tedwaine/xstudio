from xstudio.api.session.playlist.timeline import Timeline, Stack, Track, Clip

def collect_flagged_clip_shots(item=XSTUDIO.api.session.viewed_container):
    result = []
    if isinstance(item, Timeline) or isinstance(item, Stack) or isinstance(item, Track):
        if item.enabled:
            for child in item.children:
                result.extend(collect_flagged_clip_shots(child))
    elif isinstance(item, Clip):
        if item.enabled and item.item_flag != "":
            result.append([item.item_flag_colour, item.item_name])

    return result

def export_flagged_clip_shots(item=XSTUDIO.api.session.viewed_container):
    items = collect_flagged_clip_shots(item)

    result = {}
    for i in items:
        if i[0] not in result:
            result[i[0]] = set()

        result[i[0]].add(i[1])

    for i in sorted(result.keys()):
        print(i+":")
        for ii in sorted(result[i]):
            print (ii)

export_flagged_clip_shots()
