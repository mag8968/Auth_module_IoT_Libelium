package com.gaejexperiments.db;

import java.io.IOException;
import java.util.Iterator;
import java.util.Map;

import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

@SuppressWarnings("serial")
public class ReportsServlet extends HttpServlet {
	public void doGet(HttpServletRequest req, HttpServletResponse resp)
			throws IOException {
		resp.setContentType("text/html");
		String [] strResult={"","",""};
		String [] strData;
		try {
			String type = (String) req.getParameter("type");
			if (type == null) {

			} else if (type.equals("sensor")) {
				String strTagCode = (String) req.getParameter("tagcode");
				strData = DBUtils
						.getSensorValueForTagCode(strTagCode);
				strResult[0] = "["
						+ strData[0] + "]";
				strResult[1] = "["
						+ strData[1] + "]";
				strResult[2] = "["
						+ strData[2] + "]";
			}
		} catch (Exception ex) {
			strResult[0] = "<Response><Status>fail</Status><StatusDescription>"
					+ "Error in executing operation : " + ex.getMessage()
					+ "</StatusDescription></Response>";
			strResult[1] = "<Response><Status>fail</Status><StatusDescription>"
					+ "Error in executing operation : " + ex.getMessage()
					+ "</StatusDescription></Response>";
			strResult[2] = "<Response><Status>fail</Status><StatusDescription>"
					+ "Error in executing operation : " + ex.getMessage()
					+ "</StatusDescription></Response>";
		}
		String UX_Part1 = "<html><head> <meta charset=\"UTF-8\"> <title>Sensor Report</title> <script type=\"text/javascript\" src=\"https://www.google.com/jsapi\"></script> <script type=\"text/javascript\"> google.setOnLoadCallback(drawChart); function drawChart() { var data = new google.visualization.DataTable(); data.addColumn('string', 'X'); data.addColumn('number', 'Lux'); data.addRows(";
		UX_Part1+= strResult[0];
		UX_Part1+="); var options = { hAxis: { title: 'Time' }, vAxis: { title: 'Light Sensor' }}; var chart = new google.visualization.LineChart(document.getElementById('chart_div')); chart.draw(data, options); } google.load('visualization', '1', {'packages': ['annotatedtimeline', 'corechart']}); google.load(\"visualization\", \"1\", {packages: [\"corechart\"]}); google.setOnLoadCallback(drawChart2); function drawChart2() { var data = new google.visualization.DataTable(); data.addColumn('string', 'X'); data.addColumn('number', 'Decibel'); data.addRows(";
		UX_Part1+= strResult[1];
		UX_Part1+="); var options = { hAxis: { title: 'Time' }, vAxis: { title: 'Sound Sensor' }}; var chart = new google.visualization.LineChart(document.getElementById('chart_div2')); chart.draw(data, options); } </script> </head> <body> <div id=\"chart_div\" style=\"width: 1200px; height: 500px;\"></div> <div id=\"chart_div2\" style=\"width: 1200px; height: 500px;\"></div> </body> </html>";
		System.out.println(UX_Part1);
		resp.getWriter().println(UX_Part1);
	} 

}
