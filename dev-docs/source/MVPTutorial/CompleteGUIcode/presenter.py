class Presenter(object):

    def __init__(self, view, colours):
        self.view = view
        self.view.setColours(colours)

    def getPlotInfo(self):
        return str(self.view.getColour()), self.view.getFreq(), self.view.getPhase()

    def getGridLines(self):
        return self.view.getGridLines()
