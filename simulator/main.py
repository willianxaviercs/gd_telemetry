import sqlite3


def main() -> None:
    # This only verifies that the simulator can create or open a local SQLite file.
    connection = sqlite3.connect("simulator.db")
    connection.close()
    print("simulator: skeleton entrypoint")


if __name__ == "__main__":
    main()
