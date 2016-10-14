import time
import sys
import os
import io
import string
import math
import re
from datetime import datetime

if len(sys.argv) < 3:
	print 'Usage: %s [path_to_SE_xml_folder] [output_file]' % (sys.argv[0])
	sys.exit()

user_id_map = dict()
post_id_map = dict()

path_to_input_directory = sys.argv[1]
path_to_output_file = sys.argv[2]

#####################################################################
# Regex for extracting data from xml logs
#####################################################################
extract_user_row = re.compile("<row Id=\"([^\"]*)\" Reputation=\"([^\"]*)\" CreationDate=\"([^\"]*)\" DisplayName=\"([^\"]*)\"")

#id, postid, votetype, userid, create_date
extract_votes_row = re.compile("<row Id=\"([^\"]*)\" PostId=\"([^\"]*)\" VoteTypeId=\"([^\"]*)\" UserId=\"([^\"]*)\" CreationDate=\"([^\"]*)\"")

# Post Row example
#<row Id="5188" PostTypeId="1" CreationDate="2014-04-02T17:54:55.367" Score="12" ViewCount="280" Body="&lt;p&gt;Consider the following &lt;strong&gt;chess variant:&lt;/strong&gt;&lt;/p&gt;&#xA;&#xA;&lt;ul&gt;&#xA;&lt;li&gt;Black can, &lt;em&gt;once per game&lt;/em&gt;, make two moves in a row (the second move may not capture the King)&lt;/li&gt;&#xA;&lt;li&gt;To compensate for this advantage, Black gets a material handicap&lt;/li&gt;&#xA;&lt;li&gt;Otherwise, the standard rules of chess apply&lt;/li&gt;&#xA;&lt;/ul&gt;&#xA;&#xA;&lt;p&gt;The question is, &lt;strong&gt;approximately what material handicap would be needed for the game to become roughly balanced?&lt;/strong&gt; It is clear that making two consecutive moves, even only once per game, is a tremendous advantage. My guess is that at least a Queen handicap is necessary, since it is trivially easy to capture White's Queen with two consecutive moves once the pieces are out.&lt;/p&gt;&#xA;&#xA;&lt;p&gt;Has this chess variant been studied before?&lt;/p&gt;&#xA;" OwnerDisplayName="user2659" LastActivityDate="2015-02-20T11:23:43.610" Title="What material handicap would be required to compensate for a single double-move?" Tags="&lt;chess-variants&gt;" AnswerCount="2" CommentCount="17" />
extract_post_row = re.compile("<row Id=\"([^\"]*)\" PostTypeId=\"1\" (.* )?CreationDate=\"([^\"]*)\" (.* )?Owner[^=]*=\"([^\"]*)\"")
extract_reply_row = re.compile("<row Id=\"([^\"]*)\" PostTypeId=\"2\" (.* )?ParentId=\"([^\"]*)\" (.* )?CreationDate=\"([^\"]*)\" (.* )?OwnerUserId=\"([^\"]*)\" .* />")

# Comment row example
#<row Id="2" PostId="4" Score="0" Text="I've heard of people offering move advantages early in the game before, but I'm not sure what metric you'd use to determine how much one move is worth in rating terminology. Does a bonus 1 move for the first 3 moves equate to a 200 point spread?" CreationDate="2012-05-01T19:25:23.753" UserId="29" />
# Id, postId, score, text, ts, userid
extract_comment_row = re.compile("<row Id=\"([^\"]*)\" PostId=\"([^\"]*)\" Score=\"([^\"]*)\" Text=\"([^\"]*)\" CreationDate=\"([^\"]*)\" UserId=\"([^\"]*)\"")
###################################################################

TS_PATTERN = '%Y-%m-%dT%H:%M:%S.%f'
output_file = io.open(path_to_output_file, "wb")
next_node_id = 0

###################################################################
# Users.xml
###################################################################
user_file = io.open(path_to_input_directory + "/Users.xml", "rU", encoding='utf-8')

for line in user_file:
	m_user_row = extract_user_row.search(line)
	if m_user_row:
		user_id = m_user_row.group(1).strip()
		user_name = m_user_row.group(4).strip()
		user_ts = m_user_row.group(3).strip()

		epoch = int(time.mktime(time.strptime(user_ts, TS_PATTERN)))
		user_id_map[user_id] = next_node_id
		out_str = str(epoch) + " V " + str(next_node_id) + " USER \"" + re.sub("[\"]","'",user_name) + "\""
		# Increment vertex count
		next_node_id += 1
		output_file.write(out_str.encode("utf-8") + os.linesep)

user_file.close()

###################################################################
# Posts.xml
###################################################################
post_file = io.open(path_to_input_directory + "/Posts.xml", "rU", encoding='utf-8')

for line in post_file:
	m_post_row = extract_post_row.search(line)

	if m_post_row:

		post_id = m_post_row.group(1)
		post_on = m_post_row.group(3)
		post_by = m_post_row.group(5)
		post_epoch = int(time.mktime(time.strptime(post_on, TS_PATTERN)))
		post_id_map[post_id] = next_node_id
		post_vertex = str(post_epoch) + " V " + str(next_node_id) + " POST"
		output_file.write(post_vertex.encode("utf-8") + os.linesep)

		if (post_by in user_id_map):
			edge_to_user = str(post_epoch) + " E " + str(user_id_map[post_by]) + " " + str(next_node_id) + " POSTED"
			# increment vertex count
			output_file.write(edge_to_user.encode("utf-8") + os.linesep)

		next_node_id += 1


	else:
		m_reply_row = extract_reply_row.search(line)
		if m_reply_row:
			post_id = m_reply_row.group(1)
			parent_id = m_reply_row.group(3)
			post_by = m_reply_row.group(7)
			post_on = m_reply_row.group(5)
			post_epoch = int(time.mktime(time.strptime(post_on, TS_PATTERN)))
			post_id_map[post_id] = next_node_id

			if (parent_id in post_id_map and post_by in user_id_map):

				post_vertex = str(post_epoch) + " V " + str(next_node_id) + " REPLY"
				edge_to_parent = str(post_epoch) + " E " + str(next_node_id) + " " + str(post_id_map[parent_id]) + " REPLY_TO"
				edge_to_user = str(post_epoch) + " E " + str(user_id_map[post_by]) + " " + str(next_node_id) + " POSTED"
				next_node_id += 1

				output_file.write(post_vertex.encode("utf-8") + os.linesep)
				output_file.write(edge_to_parent.encode("utf-8") + os.linesep)
				output_file.write(edge_to_user.encode("utf-8") + os.linesep)

			elif (parent_id in post_id_map):

				post_vertex = str(post_epoch) + " V " + str(next_node_id) + " REPLY"
				edge_to_parent = str(post_epoch) + " E " + str(next_node_id) + " " + str(post_id_map[parent_id]) + " REPLY_TO"
				next_node_id += 1

				output_file.write(post_vertex.encode("utf-8") + os.linesep)
				output_file.write(edge_to_parent.encode("utf-8") + os.linesep)

			elif (post_by in user_id_map):

				post_vertex = str(post_epoch) + " V " + str(next_node_id) + " REPLY"
				edge_to_user = str(post_epoch) + " E " +  str(user_id_map[post_by]) + " " + str(next_node_id) + " POSTED"
				next_node_id += 1

				output_file.write(post_vertex.encode("utf-8") + os.linesep)
				output_file.write(edge_to_user.encode("utf-8") + os.linesep)

			else:
				raise SystemExit

post_file.close()

###################################################################
# Votes.xml
###################################################################
votes_file = io.open(path_to_input_directory + "/Votes.xml", "rU", encoding="utf-8")

for line in votes_file:
	m_votes_row = extract_votes_row.search(line)

	if m_votes_row:
		vote_id   = m_votes_row.group(1)
		post_id   = m_votes_row.group(2)
		vote_type = m_votes_row.group(3)
		user_id   = m_votes_row.group(4)
		vote_ts   = m_votes_row.group(5)

		epoch = int(time.mktime(time.strptime(vote_ts, TS_PATTERN)))

		if (user_id in user_id_map and post_id in post_id_map):
			out_str = str(epoch) + " E " + str(user_id_map[user_id]) +  " " + str(post_id_map[post_id]) + " VOTED_FOR"
			output_file.write(out_str.encode("utf-8")  + os.linesep)
		elif (post_id in post_id_map):
			pass
			# print "# User not found " + user_id
		elif (user_id in user_id_map):
			pass
			# print "# Post not found " + post_id

votes_file.close()

###################################################################
# comments.xml
###################################################################
comments_file = io.open(path_to_input_directory + "/Comments.xml", "rU", encoding="utf-8")

for line in comments_file:
	m_comment_row = extract_comment_row.search(line)

	if m_comment_row:
		# Id, postId, score, text, ts, userid
		vote_id    = m_comment_row.group(1)
		post_id    = m_comment_row.group(2)
		score      = m_comment_row.group(3)
		comment_ts = m_comment_row.group(5)
		user_id    = m_comment_row.group(6)

		epoch = int(time.mktime(time.strptime(comment_ts, TS_PATTERN)))

		if (user_id in user_id_map and post_id in post_id_map):

			out_str_1 = str(epoch) + " V " + str(next_node_id) + " COMMENT"
			out_str_2 = str(epoch) + " E " + str(user_id_map[user_id]) +  " " + str(next_node_id) + " MADE_COMMENT"
			out_str_3 = str(epoch) + " E " + str(next_node_id) +  " " + str(post_id_map[post_id]) + " COMMENT_ON"
			next_node_id += 1

			output_file.write(out_str_1.encode("utf-8")  + os.linesep)
			output_file.write(out_str_2.encode("utf-8")  + os.linesep)
			output_file.write(out_str_3.encode("utf-8")  + os.linesep)

		elif (post_id in post_id_map):
			pass
			# print "# User not found " + user_id
		elif (user_id in user_id_map):
			pass
			# print "# Post not found " + post_id


comments_file.close()



output_file.close()
