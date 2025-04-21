from dataclasses import dataclass
from typing import List

@dataclass
class Room:
    id: int
    name: str
    password_required: bool
    max_players: int

def load_room_file(file_path: str) -> List[Room]:
    rooms = []
    with open(file_path) as f:
        for line in f:
            fields = line.strip().split(',')
            if len(fields) >= 4:
                rooms.append(Room(
                    id=int(fields[0]),
                    name=fields[1],
                    password_required=fields[2] == '1',
                    max_players=int(fields[3])
                ))
    return rooms
