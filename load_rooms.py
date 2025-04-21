import json
import sys
from common.room_info import load_room_file

input_path = sys.argv[1] if len(sys.argv) > 1 else "rooms.lst"
rooms = load_room_file(input_path)
print(json.dumps([room.__dict__ for room in rooms]))
