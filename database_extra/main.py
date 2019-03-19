from sqlalchemy import Column, String, create_engine,\
                       Integer, ForeignKey, DECIMAL, desc
from sqlalchemy.orm import sessionmaker
from sqlalchemy.ext.declarative import declarative_base


class PLAYER(declarative_base()):
    __tablename__ = 'PLAYER'
    PLAYER_ID = Column(Integer, primary_key=True)
    TEAM_ID = Column(Integer, ForeignKey('TEAM.TEAM_ID'))
    UNIFORM_NUM = Column(Integer)
    FIRST_NAME = Column(String(32))
    LAST_NAME = Column(String(32))
    MPG = Column(Integer)
    PPG = Column(Integer)
    RPG = Column(Integer)
    APG = Column(Integer)
    SPG = Column(DECIMAL(3, 1))
    BPG = Column(DECIMAL(3, 1))

    
class TEAM(declarative_base()):
    __tablename__ = 'TEAM'
    TEAM_ID = Column(Integer, primary_key=True)
    NAME = Column(String(32))
    STATE_ID = Column(Integer, ForeignKey('STATE.STATE_ID'))
    COLOR_ID = Column(Integer, ForeignKey('COLOR.COLOR_ID'))
    WINS = Column(Integer)
    LOSSES = Column(Integer)

    
class STATE(declarative_base()):
    __tablename__ = 'STATE'
    STATE_ID = Column(Integer, primary_key=True)
    NAME = Column(String(32))


class COLOR(declarative_base()):
    __tablename__ = 'COLOR'
    COLOR_ID = Column(Integer, primary_key=True)
    NAME = Column(String(32))

def openSession():
    engine = create_engine('postgresql+psycopg2://postgres:passw0rd@localhost/ACC_BBALL')

    DBSession = sessionmaker(bind=engine)

    session = DBSession()
    return session


def query1(use_mpg, min_mpg, max_mpg, use_ppg,
           min_ppg, max_ppg, use_rpg, min_rpg,
           max_rpg, use_apg, min_apg, max_apg,
           use_spg, min_spg, max_spg, use_bpg,
           min_bpg, max_bpg):
    session = openSession()
    query = session.query(PLAYER)
    if use_mpg:
        query = query.filter(PLAYER.MPG.between(min_mpg, max_mpg))
    if use_ppg:
        query = query.filter(PLAYER.PPG.between(min_ppg, max_ppg))
    if use_rpg:
        query = query.filter(PLAYER.RPG.between(min_rpg, max_rpg))
    if use_apg:
        query = query.filter(PLAYER.APG.between(min_apg, max_apg))
    if use_spg:
        query = query.filter(PLAYER.SPG.between(min_spg, max_spg))
    if use_bpg:
        query = query.filter(PLAYER.BPG.between(min_bpg, max_bpg))
    print("PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG")
    for row in query:
        print(row.PLAYER_ID, row.TEAM_ID, row.UNIFORM_NUM,
              row.FIRST_NAME, row.LAST_NAME, row.MPG,
              row.PPG, row.RPG, row.APG, row.SPG, row.BPG)

        
def query2(UNIFORM_COLOR):
    session = openSession()
    query = session.query(COLOR, TEAM)
    print("NAME")
    for row in query.filter(COLOR.NAME == UNIFORM_COLOR)\
                    .filter(COLOR.COLOR_ID == TEAM.COLOR_ID)\
                    .all():
        print(row.TEAM.NAME)

        
def query3(TEAM_NAME):
    session = openSession()
    query = session.query(PLAYER, TEAM)
    print("FIRST_NAME LAST_NAME")
    for row in query.filter(PLAYER.TEAM_ID == TEAM.TEAM_ID)\
                    .filter(TEAM.NAME == TEAM_NAME)\
                    .order_by(desc(PLAYER.PPG))\
                    .all():
        print(row.PLAYER.FIRST_NAME, row.PLAYER.LAST_NAME)

        
def query4(TEAM_STATE, TEAM_COLOR):
    session = openSession()
    query = session.query(PLAYER, STATE, COLOR, TEAM)
    print("FIRST_NAME LAST_NAME UNIFORM_NAME")
    for row in query.filter(STATE.STATE_ID == TEAM.STATE_ID)\
                    .filter(PLAYER.TEAM_ID == TEAM.TEAM_ID)\
                    .filter(COLOR.COLOR_ID == TEAM.COLOR_ID)\
                    .filter(STATE.NAME == TEAM_STATE)\
                    .filter(COLOR.NAME == TEAM_COLOR):
        print(row.PLAYER.FIRST_NAME,
              row.PLAYER.LAST_NAME,
              row.PLAYER.UNIFORM_NUM)

        
def query5(NUM_WINS):
    session = openSession()
    query = session.query(PLAYER, TEAM)
    print("FIRST_NAME LAST_NAME NAME WINS")
    for row in query.filter(PLAYER.TEAM_ID == TEAM.TEAM_ID)\
                    .filter(TEAM.WINS > NUM_WINS):
        print(row.PLAYER.FIRST_NAME,
              row.PLAYER.LAST_NAME,
              row.TEAM.NAME,
              row.TEAM.WINS)
        

query1(1, 35, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

# query2('Red')

# query3('Duke')

# query4('NC', 'DarkBlue')

# query5(10)
