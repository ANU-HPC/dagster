

class integer_range(object):
	def __init__(self):
		self.range = []
	def add(self,e):
		for i in range(len(self.range)):
			if e>= self.range[i][0] and e<= self.range[i][1]:
				return
			if e==self.range[i][1]+1:
				self.range[i][1] += 1
				return
			if e==self.range[i][0]-1:
				self.range[i][0] -= 1
				return
		self.range.append([e,e])
	def __contains__(self,e):
		for r in self.range:
			if e>=r[0] and e<=r[1]:
				return True
		return False
